#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    TAILQ_INSERT_TAIL(&clients, client, nodes);
    return client;
}

void client_handler(Client *client)
{
    PacketHeader packet_header;
    while (1)
    {
        if (recv(client->fd, &packet_header, sizeof(PacketHeader), MSG_WAITALL) != sizeof(PacketHeader))
        {
            printf("client %i disconnected\n", client->id);
            // TODO: broadcast disconnect message
            return;
        }
        void *buffer = malloc(packet_header.length);
        if (recv(client->fd, buffer, packet_header.length, MSG_WAITALL) != packet_header.length)
        {
            printf("client %i disconnected\n", client->id);
            // TODO: broadcast disconnect message
            return;
        }
        PacketClass *class = packet_classes[packet_header.type];
        void *packet_data = class->read(buffer);
        if (class->handle_server)
        {
            class->handle_server(packet_data, client);
        }
        free(packet_data);
    }
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

void setup_server_handlers()
{
    packet_classes[SEND_MESSAGE]->handle_server = (void(*)(void*, Client *)) (void*)on_message;
}

void start_server(int port) {
    TAILQ_INIT(&clients);
    setup_server_handlers();
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
