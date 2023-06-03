#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>

#define MAX_CLIENTS 10

// this function will add a new client to the list of connected clients
void add_client_address(struct sockaddr_in adressesArray[MAX_CLIENTS], struct sockaddr_in clientAdress, int currentCLientCount)
{
    if (currentCLientCount < 9)
    {
        adressesArray[currentCLientCount] = clientAdress;
    }
    else
    {
        perror("sorry, reached maximum client count");
    }
}

void start_server(int PORT)
{
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_sockfd, MAX_CLIENTS);

    fd_set master_fds;
    FD_ZERO(&master_fds);
    FD_SET(server_sockfd, &master_fds);
    int max_fd = server_sockfd;
    int currentCLientCount = 0;
    struct sockaddr_in adressesArray[MAX_CLIENTS];
    char *clientAdress;

    while (1)
    {
        fd_set read_fds = master_fds;
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(1);
        }

        for (int fd = 0; fd <= max_fd; fd++)
        {
            if (FD_ISSET(fd, &read_fds))
            {
                if (fd == server_sockfd)
                {
                    // handle new connections
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
                    if (client_sockfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        add_client_address(adressesArray, client_addr, currentCLientCount);
                        currentCLientCount += 1;
                        printf("New client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                        FD_SET(client_sockfd, &master_fds);
                        if (client_sockfd > max_fd)
                        {
                            max_fd = client_sockfd;
                        }
                    }
                }
                else
                {
                    // handle data from a client
                    char buffer[1024], message[1024];
                    int bytes_received = recv(fd, buffer, sizeof(buffer), 0);
                    if (bytes_received == -1)
                    {
                        perror("recv");
                    }
                    else if (bytes_received == 0)
                    {
                        // connection closed by client

                        printf("Client %s was disconnected\n", inet_ntoa(adressesArray[fd].sin_addr));

                        close(fd);
                        FD_CLR(fd, &master_fds);
                    }
                    else
                    {
                        // TODO: timestamp
                        sprintf(message, "<user%i>: %s", fd, buffer);
                        int message_length = strlen(message);
                        printf(message);
                        // broadcast the received message to all clients
                        for (int i = 0; i <= max_fd; i++)
                        {
                            if (!FD_ISSET(i, &master_fds) || i == server_sockfd)
                            {
                                continue;
                            }
                            if (send(i, message, message_length + 1, 0) == -1)
                            {
                                perror("send");
                            }
                        }
                        bzero(buffer, sizeof(buffer));
                    }
                }
            }
        }
    }
    close(server_sockfd);
}
