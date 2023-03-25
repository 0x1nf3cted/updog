#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <arpa/inet.h>

#include "server.h"
#include "utils.h"


void start_server(int PORT)
{
    int bytesRead;
    struct sockaddr_in address;
    int option = 1;
    int addressLength = sizeof(address);

    int serverFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFileDescriptor == -1)
    {
        perror("Error: failed to create a socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    int socketBinding = bind(serverFileDescriptor, (struct sockaddr *)&address,
                             sizeof(address));
    if (socketBinding == -1)
    {
        printf("Error: socket bindin failed");
        close(serverFileDescriptor);
        exit(EXIT_FAILURE);
    }
    if (listen(serverFileDescriptor, 3) < 0)
    {
        printf("Listening on port:  %d", PORT);
        exit(EXIT_FAILURE);
    }
    int newSocketFileDescriptor = accept(serverFileDescriptor, (struct sockaddr *)&address,
                                         (socklen_t *)&addressLength);
    if (newSocketFileDescriptor == -1)
    {
        printf("accept");
        close(serverFileDescriptor);
        exit(EXIT_FAILURE);
    }
    printf("client joined the discussion \n");

    char buffer[1024] = {0};
    bool stop = false;
    char message[1024];
    int send_status;
    int receive_status;

    while (!stop)
    {
        receive_status = read(newSocketFileDescriptor, buffer, 1024);
        printf("Client: ");
        printf("%s", buffer);
        stop = check_if_disconnected(buffer, receive_status);

        printf("Server: ");
        fgets(message, 1024, stdin);
        send_status = send(newSocketFileDescriptor, message, strlen(message), 0);
        stop = check_if_disconnected(message, send_status);

        // memset(buffer, 0, sizeof(buffer));
        // memset(message, 0, sizeof(message));
        bzero(buffer, sizeof(buffer));
        bzero(message, sizeof(message));
    }

    close(newSocketFileDescriptor);
    shutdown(serverFileDescriptor, SHUT_RDWR);
}
