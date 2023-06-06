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

void add_message(char *message)
{
    MessageNode *node = malloc(sizeof(MessageNode));
    
    int length = strlen(message);
    node->message = malloc(length + 1);
    memcpy(node->message, message, length+1);

    TAILQ_INSERT_TAIL(&all_messages, node, nodes);
}

void send_message(char *message)
{
    /* 
     * there is still a bug here, that need to be fixed
     * sometimes when /q is sent the client disconnected
     * from the server, but other times it crashes the server
     */
    if (strcmp(message, "/q\n") == 0)
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
    wprintw(message_window.window, buffer);
    wrefresh(message_window.window);
    free(data->message);
    
    // move cursor to input_window
    wrefresh(input_window.window);
}

void setup_client_handlers()
{
    packet_classes[NOTIFY_MESSAGE]->handle_client = (void(*)(void*)) (void*)on_notify_message;
}


void start_client(char *address, int port)
{
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
