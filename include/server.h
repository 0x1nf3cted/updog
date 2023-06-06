#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/queue.h>
#include <pthread.h>

typedef struct ClientStruct {
    int fd, id;
    char *displayName;
    struct sockaddr_in address;
    pthread_t thread;
    // last heartbeat time
    // username
    // ...
    TAILQ_ENTRY(ClientStruct) nodes;
} Client;

typedef TAILQ_HEAD(AllClients, ClientStruct) ClientList;

void start_server(int PORT);

#endif /* SERVER_H */
