#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>

#include "client.h"

void start_client(char *adress, int SERVER_PORT)
{

    int valread, client_sockfd;
    struct sockaddr_in server_address;
    if ((client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, adress, &server_address.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    int status = connect(client_sockfd, (struct sockaddr *)&server_address,
                         sizeof(server_address));
    if (status == -1)
    {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }

    char buffer[1024] = {0};
    bool stop = false;
    char message[1024];

    while (!stop)
    {

        printf("Client: ");
        fgets(message, 1024, stdin);

        send(client_sockfd, message, strlen(message), 0);

        read(client_sockfd, buffer, 1024);
        printf("Server: ");
        printf("%s\n", buffer);
        if (strcmp(message, "/q") == 0)
        {
            stop = true;
        }
        memset(buffer, 0, sizeof(buffer));
        memset(message, 0, sizeof(message));
    }

    close(client_sockfd);
}
