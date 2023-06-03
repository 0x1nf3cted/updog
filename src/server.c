#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define MAX_CLIENTS 10

typedef struct {
    struct sockaddr_in client_addr;
    int sockfd;
    char pseudo[20];
} Client;

// This function retrieves the current time.
const char *get_time() {
    time_t now;
    time(&now);
    struct tm tm_now = *localtime(&now);
    static char s_now[sizeof "HH::MM::SS"];
    strftime(s_now, sizeof s_now, "%H:%M:%S", &tm_now);
    return s_now;
}

// this function will send to all the client, the message written
void send_message_to_all_clients(Client clientsArray[MAX_CLIENTS],
        int senderSockfd, const char *message,
        int currentClientCount) {
    for (int i = 0; i < currentClientCount; i++) {
        if (clientsArray[i].sockfd != senderSockfd) {
            char buffer[1024];
            sprintf(buffer, "[%s] say: %s\n", clientsArray[i].pseudo, message);
            send(clientsArray[i].sockfd, buffer, strlen(buffer), 0);
        }
    }
}

// this function will add a new client to the list of connected clients
void add_client(Client clientsArray[MAX_CLIENTS], int sockfd,
        const char *pseudo, struct sockaddr_in client_addr,
        int *currentClientCount) {
    if (*currentClientCount < MAX_CLIENTS) {
        Client newClient;
        newClient.sockfd = sockfd;
        newClient.client_addr = client_addr;
        strcpy(newClient.pseudo, pseudo);
        clientsArray[*currentClientCount] = newClient;
        (*currentClientCount)++;
    } else {
        perror("Désolé, nombre maximum de clients atteint");
    }
}

void start_server(int port) {
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_sockfd, MAX_CLIENTS);

    fd_set master_fds;
    FD_ZERO(&master_fds);
    FD_SET(server_sockfd, &master_fds);
    int max_fd = server_sockfd;
    int currentClientCount = 0;
    Client clientsArray[MAX_CLIENTS];

    while (1) {
        fd_set read_fds = master_fds;
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == server_sockfd) {
                    // handle new connections
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int client_sockfd = accept(
                            server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
                    if (client_sockfd == -1) {
                        perror("accept");
                    } else {
                        char buffer[1024];
                        int bytes_received = recv(client_sockfd, buffer, sizeof(buffer), 0);
                        if (bytes_received > 0) {
                            buffer[bytes_received] = '\0';
                            printf("New client connected from %s:%d with pseudo: %s\n",
                                    inet_ntoa(client_addr.sin_addr),
                                    ntohs(client_addr.sin_port), buffer);
                            add_client(clientsArray, client_sockfd, buffer, client_addr,
                                    &currentClientCount);
                            FD_SET(client_sockfd, &master_fds);
                            if (client_sockfd > max_fd) {
                                max_fd = client_sockfd;
                            }
                        } else {
                            perror("recv");
                            close(client_sockfd);
                        }
                    }
                } else {
                    // handle data from a client
                    char buffer[1024];
                    int bytes_received = recv(fd, buffer, sizeof(buffer), 0);
                    if (bytes_received == -1) {
                        perror("recv");
                    } else if (bytes_received == 0) {
                        // connection closed by client
                        struct sockaddr_in client_addr;
                        socklen_t client_addr_len = sizeof(client_addr);
                        getpeername(fd, (struct sockaddr *)&client_addr, &client_addr_len);

                        for (int i = 0; i < currentClientCount; i++) {
                            if (clientsArray[i].sockfd == fd) {
                                printf("Client %s:%d disconnected\n", clientsArray[i].pseudo,
                                        ntohs(client_addr.sin_port));
                                close(fd);
                                FD_CLR(fd, &master_fds);
                                for (int j = i; j < currentClientCount - 1; j++) {
                                    clientsArray[j] = clientsArray[j + 1];
                                }
                                currentClientCount--;
                                break;
                            }
                        }
                    } else {
                        const char *current_time = get_time();
                        buffer[bytes_received] = '\0';
                        for (int i = 0; i < currentClientCount; i++) {
                            if (clientsArray[i].sockfd == fd) {
                                // broadcast the received message to all client
                                printf("At %s (%s) (Ip %s:%d) say : %s\n", current_time,
                                        clientsArray[i].pseudo,
                                        inet_ntoa(clientsArray[i].client_addr.sin_addr),
                                        ntohs(clientsArray[i].client_addr.sin_port), buffer);
                                send_message_to_all_clients(clientsArray,
                                        clientsArray[i].sockfd, buffer,
                                        currentClientCount);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    close(server_sockfd);
}
