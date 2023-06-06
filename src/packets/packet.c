#include "protocol.h"
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>

void send_message_packet(int fd, char *message)
{
    send_packet(fd, SEND_MESSAGE, message);
}

void notify_message_packet(int fd, int user_id, char *message)
{
    send_packet(fd, NOTIFY_MESSAGE, user_id, message);
}

void notify_disconnect_packet(int fd, int user_id)
{
    send_packet(fd, NOTIFY_DISCONNECT, user_id);
}

void notify_connect_packet(int fd, int user_id)
{
    send_packet(fd, NOTIFY_CONNECT, user_id);
}

void heartbeat_packet(int fd)
{
    send_packet(fd, HEARTBEAT);
}

PACKET_TYPES(CLASS, )

#define CLASS_NAME(class) &class##_CLASS
PacketClass *packet_classes[] = {
    PACKET_TYPES(CLASS_NAME, COMMA)
};


// UNSAFE function, create your own wrapper for proper type annotations
// void *myPacket(int param) { return createPacket(MY_PACKET, param); }
void send_packet(int sockfd, PacketType type, ...)
{
    PacketClass *class = packet_classes[type];
    va_list args;
    va_start(args, type);
    int size = class->length(args);
    va_end(args);
    int send_size = sizeof(PacketHeader) + size;
    void *data = malloc(send_size);
    PacketHeader *header = data;
    header->length = size;
    header->type = type;
    void *writeData = &header[1];
    va_start(args, type);
    class->insert(writeData, args);
    va_end(args);

    if (send(sockfd, data, send_size, 0) == -1)
    {
        // finalize();
        perror("send failed");
        exit(EXIT_FAILURE);
    }
}

inline int STRING_LENGTH(char *string)
{
    return strlen(string) + sizeof(uint16_t);
}

void STRING_INSERT(char ***buffer, char *string)
{
    int count = strlen(string);
    uint16_t **length = (void *) buffer;
    **length = count;
    (*length)++;
    char **buffer_string = (void *)buffer;
    for (int i = 0; i < count; i++) {
        **buffer_string = string[i];
        (*buffer_string)++;
    }
}

char *STRING_READ(char ***data)
{
    uint16_t **length = (void*) data;
    int size = **length;
    (*length)++;
    char **buffer_string = (void *) data;
    char *string = malloc(size + 1);
    for (int i = 0; i < size; i++)
    {
        string[i] = **buffer_string;
        (*buffer_string)++;
    }
    string[size] = 0;
    return string;
}

inline int U8_LENGTH(uint8_t data)
{
    return sizeof(uint8_t);
}

void U8_INSERT(uint8_t **buffer, uint8_t data)
{
    **buffer = data;
    (*buffer)++;
}

uint16_t U8_READ(uint8_t **buffer)
{
    uint8_t value = **buffer;
    (*buffer)++;
    return value;
}

inline int U16_LENGTH(uint16_t data)
{
    return sizeof(uint16_t);
}

void U16_INSERT(uint16_t **buffer, uint16_t data)
{
    **buffer = data;
    (*buffer)++;
}

uint16_t U16_READ(uint16_t **buffer)
{
    uint16_t value = **buffer;
    (*buffer)++;
    return value;
}
