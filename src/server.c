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

void handle_signal()
{
    printf("chat has ended\n");
    exit(1);
}

int start_server(int PORT)
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
        exit(EXIT_FAILURE);
    }
    printf("client joined the discussion \n");

    char buffer[1024] = {0};
    bool stop = false;
    char message[1024];

    while (!stop)
    {
        bytesRead = recv(newSocketFileDescriptor, buffer, 1024,0);
        if(bytesRead == -1){
            stop = true;
        }
        printf("Client: ");
        printf("%s\n", buffer);

        printf("Server: ");
        scanf("%[^\n]", message);

        send(newSocketFileDescriptor, message, strlen(message), 0);

        if (strcmp(buffer, "/q") == 0)
        {
            stop = true;
        }
        memset(buffer, 0, sizeof(buffer));
        memset(message, 0, sizeof(message));
    }
    free(buffer);
    free(message);

    close(newSocketFileDescriptor);
    shutdown(serverFileDescriptor, SHUT_RDWR);
    return 0;
}
