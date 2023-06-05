#include "protocol.h"
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>

extern PacketClass PACKET_TYPES(, _PACKET_CLASS, COMMA);

PacketClass *packet_classes[] = {
    PACKET_TYPES(&, _PACKET_CLASS, COMMA)
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

void STRING_LENGTH(int *size, uintptr_t string)
{
    *size += strlen((char *) string) + sizeof(uint16_t);
}

void STRING_INSERT(void **buffer, uintptr_t data)
{
    char *string = (char *)data;
    int count = strlen(string);
    uint16_t **length = (void *) buffer;
    **length = count;
    *buffer += sizeof(uint16_t);
    char **write = (void *) buffer;
    for (int i = 0; i < count; i++) {
        (*write)[i] = string[i];
    }
    *write += count;
}

void STRING_READ(uint8_t **buffer, uintptr_t **data)
{
    int size = *((uint16_t *)*buffer);
    *buffer += sizeof(uint16_t);
    char *string = malloc(size + 10);
    for (int i = 0; i < size; i++)
    {
        string[i] = *((char *)*buffer);
        (*buffer)++;
    }
    string[size] = 0;
    **data = (uintptr_t)(void *)string;
    (*data)++;
}

void U8_LENGTH(int *size, uintptr_t data)
{
    *size += sizeof(uint8_t);
}

void U8_INSERT(uint8_t **buffer, uintptr_t data)
{
    **buffer = (uint8_t) data;
    *buffer++;
}

void U16_LENGTH(int *size, uintptr_t data)
{
    *size += sizeof(uint8_t);
}

void U16_INSERT(uint16_t **buffer, uintptr_t data)
{
    **buffer = (uint16_t) data;
    *buffer++;
}

void U16_READ(void **buffer, void ***data)
{
    //TODO
    //uint16_t value = *((uint16_t *)*buffer);
    //*buffer += sizeof(uint16_t);
    //**data = (void*)(uintptr_t) value;
    //*data++;
}
