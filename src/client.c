#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>

#include "client.h"
#include "utils.h"

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
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }

    int status = connect(client_sockfd, (struct sockaddr *)&server_address,
                         sizeof(server_address));
    if (status == -1)
    {
        printf("\nConnection Failed \n");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }

    char buffer[1024] = {0};
    bool stop = false;
    char message[1024];
    int send_status;
    int receive_status;

    while (!stop)
    {

        printf("Client: ");
        fgets(message, 1024, stdin);
        send_status = send(client_sockfd, message, strlen(message), 0);
        // stop = check_if_disconnected(message, send_status);
        stop = strcmp(message, "/q");

        receive_status = read(client_sockfd, buffer, 1024);
        printf("Server: ");
        printf("%s", buffer);
        stop = check_if_disconnected(buffer, receive_status);

        memset(buffer, 0, sizeof(buffer));
        memset(message, 0, sizeof(message));
    }

    close(client_sockfd);
}
