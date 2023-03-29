#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>

void start_client(char *address, int port)
{
    // we create the socket
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0)
    {
        perror("address conversion failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char buffer[1024] = {0};
    char message[1024] = {0};
    /*this code will bascially use non blocking socket functions to
    read incoming messages if there are any and to send messages to the server, the server will then broadcast
    the messages to all the connected clients, except the sender
    */
    while (1)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        if (select(sockfd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select failed");
            exit(EXIT_FAILURE);
        }

        /* we check if there is some available data to read
        recvfrom allow you to get the ip of the sender
        */
        if (FD_ISSET(sockfd, &read_fds))
        {
            int bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);
            if (bytes_received <= 0)
            {
                printf("Disconnected from server.\n");
                break;
            }
            printf("user%d %s", ntohs(client_addr.sin_port), buffer); // there is a bug here, the port is not the same as the adress port of the client
            memset(buffer, 0, sizeof(buffer));
        }

        /* we check if we are ready to send data
         */

        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            if (fgets(message, sizeof(message), stdin) == NULL)
            {
                break;
            }
            /* there is still a bug here, that need to be fixed
           sometimes when /q is sent the client disconnect from the server, but other times it crashes the server
           */
            if (strcmp(message, "/q\n") == 0)
            {
                break;
            }
            if (send(sockfd, message, strlen(message), 0) == -1)
            {
                perror("send failed");
                exit(EXIT_FAILURE);
            }
            memset(message, 0, sizeof(message));
        }
    }

    close(sockfd);
}