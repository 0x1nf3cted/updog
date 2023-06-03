#include <arpa/inet.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define TIMEOUT_DURATION 60

typedef struct {
    int width, height;
    WINDOW *border_window, *window;
} BorderedWindow;

BorderedWindow message_window, input_window;
int sockfd;

void init_bordered_window(BorderedWindow *window, int x, int y, int width, int height) {
    window->border_window = newwin(height, width, y, x);
    box(window->border_window, 0, 0);
    wrefresh(window->border_window);
    window->window = newwin(height - 2, width - 2, y + 1, x + 1);
    scrollok(window->window, true);
    wrefresh(window->window);
}

void initialize_ncurses() {
    initscr();
    refresh();
    init_bordered_window(&message_window, 0, 0, COLS, LINES - 5);
    init_bordered_window(&input_window, 0, LINES - 5, COLS, 5);
    // TODO: reconstruct the windows if the console gets resized
}

void finalize() {
    close(sockfd);
    endwin();
}

void start_client(char *address, int port) {
    initialize_ncurses();
    // we create the socket
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        finalize();
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0) {
        finalize();
        perror("address conversion failed");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_address,
                sizeof(server_address)) == -1) {
        finalize();
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024] = {0};
    char message[1024] = {0};
    char username[1024] = {0};
    int message_position = 0;

    int username_ok = 0;

    wprintw(message_window.window, "Enter your username:\n");
    wrefresh(message_window.window);

    while (username_ok == 0) {
        werase(input_window.window);
        wrefresh(input_window.window);
        wscanw(input_window.window, "%s", username);

        if (send(sockfd, username, strlen(username), 0) == -1) {
            wprintw(message_window.window,
                    "Failed to send username. Please try again.\n");
            wrefresh(message_window.window);
            memset(username, '\0', sizeof(username));
        } else {
            username_ok = 1;
        }
    }

    wprintw(message_window.window, "Your username is %s\n", username);
    wrefresh(message_window.window);

    werase(input_window.window);
    wrefresh(input_window.window);

    /*this code will bascially use non blocking socket functions to
      read incoming messages if there are any and to send messages to the
      server, the server will then broadcast the messages to all the connected
      clients, except the sender
      */
    time_t start_time = time(NULL);  // Get the starting timestamp

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_DURATION;
        timeout.tv_usec = 0;

        if (select(sockfd + 1, &read_fds, NULL, NULL, &timeout) == -1) {
            finalize();
            perror("select failed");
            exit(EXIT_FAILURE);
        }

        time_t current_time = time(NULL);  // Get the current timestamp
        time_t elapsed_time =
            current_time - start_time;  // Calculate the elapsed time

        // Check if the elapsed time exceeds the timeout duration
        if (elapsed_time >= TIMEOUT_DURATION) {
            printf(
                    "Client is inactive for one minute. Client is ejected from the "
                    "server.\n");
            break;
        }
        /* we check if there is some available data to read
           recvfrom allow you to get the ip of the sender
           */
        if (FD_ISSET(sockfd, &read_fds)) {
            int bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                    (struct sockaddr *)&client_addr, &addr_len);
            if (bytes_received <= 0) {
                printf("Disconnected from server.\n");
                break;
            }
            wprintw(message_window.window, buffer);
            wprintw(input_window.window, "\n");

            // there is a bug here, the port is not the same as the adress port of the
            // client
            // TODO: add port or other identifier to the message on the server, as
            // client_addr.sin_port is the server connection port
            wrefresh(message_window.window);
            // move cursor to input_window
            wrefresh(input_window.window);
            memset(buffer, 0, sizeof(buffer));
            start_time = current_time;
        }

        /* we check if we are ready to send data
        */

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char c = wgetch(input_window.window);
            if (c == '\b' || c == 127) {
                wprintw(input_window.window, "\b\b  \b\b");
                if (message_position != 0) {
                    wprintw(input_window.window, "\b  \b\b");
                    message[message_position--] = 0;
                }
                wrefresh(input_window.window);
            } else {
                message[message_position++] = c;
            }
            if (c != '\n') {
                continue;
            }

            message[message_position] = 0;

            werase(input_window.window);
            wprintw(message_window.window, "you: %s", message);
            wrefresh(message_window.window);
            wprintw(input_window.window, " \b");
            wrefresh(input_window.window);
            /* there is still a bug here, that need to be fixed
               sometimes when /q is sent the client disconnect from the server, but other
               times it crashes the server
               */
            if (strcmp(message, "/q\n") == 0) {
                break;
            }
            if (send(sockfd, message, strlen(message), 0) == -1) {
                perror("send failed");
                exit(EXIT_FAILURE);
            }
            memset(message, 0, sizeof(message));
            message_position = 0;
            start_time = current_time;
            elapsed_time = 0;
        }
    }
    finalize();
}
