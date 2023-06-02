#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/queue.h>

typedef struct {
    int width, height;
    WINDOW *border_window, *window;
} BorderedWindow;

typedef struct MessageNodeStruct {
    char *message;
    TAILQ_ENTRY(MessageNodeStruct) nodes;
} MessageNode;

typedef TAILQ_HEAD(SentMessages, MessageNodeStruct) MessageQueue;

extern MessageQueue all_messages;

extern BorderedWindow message_window, input_window;

extern void send_message(char *text);
extern void finalize();
extern void *TUI_main(void *vargp);
extern void start_client(char *adress, int SERVER_PORT);

#endif /* CLIENT_H */
