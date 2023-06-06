#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "protocol.h"

ClientList clients;

// This function retrieves the current time.
const char *get_time() {
    time_t now;
    time(&now);
    struct tm tm_now = *localtime(&now);
    static char s_now[sizeof "HH::MM::SS"];
    strftime(s_now, sizeof s_now, "%H:%M:%S", &tm_now);
    return s_now;
}

// this function will add a new client to the list of connected clients
Client *create_client(int sockfd, struct sockaddr_in client_addr) {
    Client *client = malloc(sizeof(Client));
    client->fd = sockfd;
    client->id = sockfd; // TODO
    client->address = client_addr;

    time(&client->last_heartbeat);
    
    Client *current_client;
    TAILQ_FOREACH(current_client, &clients, nodes)
    {
        notify_connect_packet(current_client->fd, client->id);
    }

    TAILQ_INSERT_TAIL(&clients, client, nodes);
    
    return client;
}

void disconnect_client(Client *disconnected_client)
{
    TAILQ_REMOVE(&clients, disconnected_client, nodes);
    Client *client;
    TAILQ_FOREACH(client, &clients, nodes)
    {
        notify_disconnect_packet(client->fd, disconnected_client->id);
    }
    close(disconnected_client->fd);
    // TODO: free client name, ...
    free(disconnected_client);
}

void client_handler(Client *client)
{
    PacketHeader packet_header;
    printf("client %i connected\n", client->id);
   
    while (1)
    {
        if (recv(client->fd, &packet_header, sizeof(PacketHeader), MSG_WAITALL) != sizeof(PacketHeader))
        {
            printf("client %i disconnected\n", client->id);
            break;
        }
        void *buffer = malloc(packet_header.length);
        if (recv(client->fd, buffer, packet_header.length, MSG_WAITALL) != packet_header.length)
        {
            printf("client %i disconnected\n", client->id);
            break;
        }
        if (packet_header.type >= PACKET_MAX) {
            printf("client %i protocol error: unknown packet type\n");
            break;
        }
        PacketClass *class = packet_classes[packet_header.type];
        void *packet_data = class->read(buffer);
        if (class->handle_server)
        {
            class->handle_server(packet_data, client);
        }
        free(packet_data);
    }
    disconnect_client(client);
}

void on_message(SEND_MESSAGE_DATA *data, Client *sender)
{
    printf("Client %i: %s\n", sender->id, data->message);
    Client *client;
    TAILQ_FOREACH(client, &clients, nodes)
    {
        notify_message_packet(client->fd, sender->id, data->message);
    }
    free(data->message);
}

void on_heartbeat(HEARTBEAT_DATA *data, Client *client)
{
    time(&client->last_heartbeat);
}

void setup_server_handlers()
{
    packet_classes[SEND_MESSAGE]->handle_server = (void(*)(void*, Client *)) (void*)on_message;
    packet_classes[HEARTBEAT]->handle_server = (void(*)(void*, Client *)) (void*)on_heartbeat;
}

void check_heartbeats()
{
    time_t current_time;
    time(&current_time);
    printf("checking heartbeats...\n");

    Client *client;
    TAILQ_FOREACH(client, &clients, nodes)
    {
        if (current_time - client->last_heartbeat > PULSE_TIME) {
            pthread_cancel(client->thread);
            disconnect_client(client);
        }
    }
}

void setup_heartbeat_check()
{
    struct sigevent event;
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = check_heartbeats;
    event.sigev_notify_attributes = NULL;

    timer_t timer;
    if (timer_create(CLOCK_MONOTONIC, &event, &timer) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    struct itimerspec timer_spec;
    timer_spec.it_value.tv_sec = 2*PULSE_TIME;
    timer_spec.it_value.tv_nsec = 0;
    timer_spec.it_interval.tv_sec =  2*PULSE_TIME;
    timer_spec.it_interval.tv_nsec = 0;

    if (timer_settime(timer, 0, &timer_spec, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}

void start_server(int port) {
    TAILQ_INIT(&clients);
    setup_server_handlers();
    setup_heartbeat_check();
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_sockfd, 10);

    fd_set master_fds;
    FD_ZERO(&master_fds);
    FD_SET(server_sockfd, &master_fds);
    
    while (1)
    {
        fd_set read_fds = master_fds;
        if (select(server_sockfd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(1);
        }
        if (FD_ISSET(server_sockfd, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_sockfd == -1)
            {
                perror("accept");
                continue;
            }
            Client *client = create_client(client_sockfd, client_addr);
            pthread_create(&client->thread, NULL, (void *)client_handler, client);
        }
    }
    close(server_sockfd);
}
