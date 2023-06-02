#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ncurses.h>

typedef struct {
    int width, height;
    WINDOW *border_window, *window;
} BorderedWindow;

extern BorderedWindow message_window, input_window;

extern void sendMessage(char *text);
extern void finalize();
extern void *TUI_main(void *vargp);
extern void start_client(char *adress, int SERVER_PORT);

#endif /* CLIENT_H */
