#define _GNU_SOURCE

#include <arpa/inet.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h> 
#include <pthread.h>
#include <signal.h>
#include "client.h"
#include "protocol.h"

#define TIMEOUT_DURATION 60

BorderedWindow message_window, input_window;
int sockfd;
MessageQueue all_messages;

// dont't free messsage after this
void add_message(char *message)
{
    MessageNode *node = malloc(sizeof(MessageNode));
    node->message = message;
    TAILQ_INSERT_TAIL(&all_messages, node, nodes);

    wprintw(message_window.window, message);
    wrefresh(message_window.window);

    wrefresh(input_window.window);
}

void send_message(char *message)
{
    if (strcmp(message, "/q") == 0)
    {
        finalize();
        perror("exting on user request");
        exit(EXIT_SUCCESS);
    }
    send_message_packet(sockfd, message);
}

void finalize()
{
    close(sockfd);
    endwin();
}

void on_notify_message(NOTIFY_MESSAGE_DATA *data)
{
    char *buffer;
    asprintf(&buffer, "<User%i>: %s\n", data->user_id, data->message);
    add_message(buffer);
}

void on_notify_connect(NOTIFY_DISCONNECT_DATA *data)
{
    char *buffer;
    asprintf(&buffer, "User%i connected\n", data->user_id);
    add_message(buffer);
}

void on_notify_disconnect(NOTIFY_DISCONNECT_DATA *data)
{
    char *buffer;
    asprintf(&buffer, "User%i disconnected\n", data->user_id);
    add_message(buffer);
}

void setup_client_handlers()
{
    packet_classes[NOTIFY_MESSAGE]->handle_client = (void(*)(void*)) (void*)on_notify_message;
    packet_classes[NOTIFY_DISCONNECT]->handle_client = (void(*)(void*)) (void*)on_notify_disconnect;
    packet_classes[NOTIFY_CONNECT]->handle_client = (void(*)(void*)) (void*)on_notify_connect;
}

void do_heartbeat()
{
    heartbeat_packet(sockfd);
}

void setup_heartbeat()
{
    struct sigevent event;
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = do_heartbeat;
    event.sigev_notify_attributes = NULL;

    timer_t timer;
    if (timer_create(CLOCK_MONOTONIC, &event, &timer) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    struct itimerspec timer_spec;
    timer_spec.it_value.tv_sec = PULSE_TIME;
    timer_spec.it_value.tv_nsec = 0;
    timer_spec.it_interval.tv_sec = PULSE_TIME;
    timer_spec.it_interval.tv_nsec = 0;

    if (timer_settime(timer, 0, &timer_spec, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}


void start_client(char *address, int port)
{
    setup_heartbeat();
    setup_client_handlers();
    TAILQ_INIT(&all_messages);

    // create the socket
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        finalize();
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0)
    {
        finalize();
        perror("address conversion failed");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        finalize();
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024] = {0};
    pthread_t tui_tread_id;
    pthread_create(&tui_tread_id, NULL, TUI_main, NULL);
    
    // SIGWINCH is handeled and processed by ncurses, don't want it in any other threads
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGWINCH);
    sigprocmask(SIG_BLOCK, &sigmask, NULL);
    

    PacketHeader packet_header;
    while (1)
    {
        if (recv(sockfd, &packet_header, sizeof(PacketHeader), MSG_WAITALL) != sizeof(PacketHeader))
        {
            break;
        }
        void *buffer = malloc(packet_header.length);
        if (recv(sockfd, buffer, packet_header.length, MSG_WAITALL) != packet_header.length)
        {
            break;
        }
        PacketClass *class = packet_classes[packet_header.type];
        void *packet_data = class->read(buffer);
        if (class->handle_client)
        {
            class->handle_client(packet_data);
        }
        free(packet_data);
    }
    finalize();
    exit(EXIT_SUCCESS);
}
