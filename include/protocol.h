#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include "server.h"

// a client should send a HEARTBEAT every PULSE_TIME secons
#define PULSE_TIME 5

#define PACKET_TYPES(T, s)                                      \
    T(SEND_MESSAGE) s                                           \
    T(NOTIFY_MESSAGE) s                                         \
    T(NOTIFY_DISCONNECT) s                                      \
    T(NOTIFY_CONNECT) s                                         \
    T(HEARTBEAT)

#define SEND_MESSAGE_PACKET(T)                                  \
    T(STRING,   message);

extern void send_message_packet(int fd, char *message);

#define NOTIFY_MESSAGE_PACKET(T)                                \
    T(U16,      user_id);                                       \
    T(STRING,   message);

extern void notify_message_packet(int fd, int user_id, char *message);

#define NOTIFY_DISCONNECT_PACKET(T)                                \
    T(U16,      user_id);

extern void notify_disconnect_packet(int fd, int user_id);

#define NOTIFY_CONNECT_PACKET(T)                                \
    T(U16,      user_id);

extern void notify_connect_packet(int fd, int user_id);

#define HEARTBEAT_PACKET(T)

extern void heartbeat_packet(int fd);

#define DATATYPES(T)                                            \
    T(U16, uint16_t)                                            \
    T(STRING, char *)

#define DATATYPE_DEF(name, type) typedef type name;
DATATYPES(DATATYPE_DEF)

#define COMMA ,

#define IDENT(x) x
enum {
    PACKET_TYPES(IDENT, COMMA),
    PACKET_MAX
};

typedef uint8_t PacketType;

#define STRUCT_LINE(type, name) type name
#define STRUCT(class)                                           \
    typedef struct { class##_PACKET(STRUCT_LINE) } class##_DATA;
PACKET_TYPES(STRUCT, )

typedef struct {
    uint32_t length;
    PacketType type;
} __attribute__((packed)) PacketHeader;

typedef struct {
    int   (*length)(va_list);
    void  (*insert)(void *, va_list);
    void *(*read)(void *);
    void  (*handle_server)(void *, Client *);
    void  (*handle_client)(void *);
    void  (*destroy)(void *);
} PacketClass;

extern PacketClass *packet_classes[];

#define T_CALL_LENGTH(type, name) length += type##_LENGTH((type)va_arg(args, uintptr_t));
#define LENGTH_FUNCTION(class)                              \
    int class##_LENGTH(va_list args) {                      \
        int length = 0;                                     \
        class##_PACKET(T_CALL_LENGTH);                      \
        return length;                                      \
    }

#define T_CALL_INSERT(type, name) type##_INSERT((type**)(void*)&buffer, (type)va_arg(args, uintptr_t));
#define INSERT_FUNCTION(class)                              \
    void class##_INSERT(void *buffer, va_list args) {       \
        class##_PACKET(T_CALL_INSERT)                       \
    }

#define T_CALL_READ(type, name) data->name = type##_READ((type**)(void*)&buffer)
#define READ_FUNCTION(class)                                \
    void *class##_READ(void *buffer) {                      \
        class##_DATA *data = malloc(sizeof(class##_DATA));  \
        class##_PACKET(T_CALL_READ);                        \
        return data;                                        \
    }

#define CLASS(class)                                        \
    LENGTH_FUNCTION(class)                                  \
    INSERT_FUNCTION(class)                                  \
    READ_FUNCTION(class)                                    \
    PacketClass class##_CLASS = {                           \
        .length = class##_LENGTH,                           \
        .insert = class##_INSERT,                           \
        .read = class##_READ,                               \
    };

#define DATATYPE_FUNCTIONS(name, type)                      \
    extern int name##_LENGTH(type);                         \
    extern void name##_INSERT(type **, type);               \
    extern type name##_READ(type**);

DATATYPES(DATATYPE_FUNCTIONS)

extern void send_packet(int sockfd, PacketType type, ...);

#endif /* PROTOCOL_H */
