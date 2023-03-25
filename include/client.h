#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


void start_client(char *adress, int SERVER_PORT);

#endif /* CLIENT_H */
