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
    int send_status = 0;
    int last_send_status = 0;
    bool sent = false;
    int receive_status;

    while (!stop)
    {
        /* Print the client messages */
        printf("Client: ");
        fgets(message, 1024, stdin);

        /* Save the message status */
        last_send_status = send_status;
        send_status = send(client_sockfd, message, strlen(message), 0);

        /* If the last message status is different than the status of
        the message that we just sent, then it was successfully sent. */
        if (send_status != last_send_status) {
          sent = true;
        }
         
        // stop = check_if_disconnected(message, send_status);

        /* If the user successfully sent "/q", then, disconnect him */
        if (strcmp(message, "/q\n") == 0 && sent == true) {
          stop = 0;
          break;
        }

        /* Save the status of the received message */
        receive_status = read(client_sockfd, buffer, 1024);

        /* Print the server messages */
        printf("Server: ");
        printf("%s", buffer);

        stop = check_if_disconnected(buffer, receive_status);

        memset(buffer, 0, sizeof(buffer));
        memset(message, 0, sizeof(message));
    }

    close(client_sockfd);
}
