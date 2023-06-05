#include "protocol.h"
#include <stdarg.h>

typedef struct {
    char *message;
} __attribute__((packed))  SendMessageData;

#define SEND_MESSAGE_PACKET(x)      \
    STRING##x;

CLASS(SEND_MESSAGE_PACKET, SendMessageData)

void send_message_packet(int sockfd, char *message) { send_packet(sockfd, SEND_MESSAGE, message); }

typedef struct {
    uintptr_t userId;
    char *message;
} __attribute__((packed))  NotifyMessageData;

#define NOTIFY_MESSAGE_PACKET(x)    \
    U16##x;                         \
    STRING##x;

CLASS(NOTIFY_MESSAGE_PACKET, NotifyMessageData)


