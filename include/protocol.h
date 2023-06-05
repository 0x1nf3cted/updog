#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>

#define COMMA ,

#define PACKET_TYPES(x, y, s)                                \
    x  SEND_MESSAGE##y s                                     \
    x  NOTIFY_MESSAGE##y

typedef enum : uint8_t {
    PACKET_TYPES(, , COMMA)
} PacketType;

typedef struct {
    uint32_t length;
    PacketType type;
} __attribute__((packed)) PacketHeader;

typedef struct {
    int   (*length)(va_list);
    void  (*insert)(void *, va_list);
    void *(*read)(void *);
    void  (*handle)(void *);
    void  (*destroy)(void *);
} PacketClass;

extern PacketClass *packet_classes[];

#define LENGTH_FUNCTION(class)                              \
    int class##_LENGTH(va_list args) {                      \
        int length = 0;                                     \
        class(_LENGTH(&length, va_arg(args, uintptr_t)));   \
        return length;                                      \
    }

#define INSERT_FUNCTION(class)                              \
    void class##_INSERT(void *buffer, va_list args) {       \
        void *write = buffer;                               \
        class(_INSERT((void *)&buffer, va_arg(args, uintptr_t)));    \
    }

#define READ_FUNCTION(class, structure)                     \
    void *class##_READ(uint8_t *buffer) {                   \
        uintptr_t *data = malloc(sizeof(structure));        \
        uintptr_t *write = data;                            \
        class(_READ(&buffer, &write));                      \
        return data;                                        \
    }

#define CLASS(class, structure)                             \
    LENGTH_FUNCTION(class)                                  \
    INSERT_FUNCTION(class)                                  \
    READ_FUNCTION(class, structure)                         \
    PacketClass class##_CLASS = {                           \
        .length = class##_LENGTH,                           \
        .insert = class##_INSERT,                           \
        .read = class##_READ,                               \
    };

extern void STRING_LENGTH(int *size, uintptr_t string);
extern void STRING_INSERT(void **buffer, uintptr_t data);
extern void STRING_READ(uint8_t **buffer, uintptr_t **data);

extern void U8_LENGTH(int *size, uintptr_t data);
extern void U8_INSERT(uint8_t **buffer, uintptr_t data);

extern void U16_LENGTH(int *size, uintptr_t data);
extern void U16_INSERT(uint16_t **buffer, uintptr_t data);
// extern void U16_READ(void **buffer, void ***data);

extern void send_packet(int sockfd, PacketType type, ...);

#endif /* PROTOCOL_H */
