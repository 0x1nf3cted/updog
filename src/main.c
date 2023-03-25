#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "server.h"
#include "client.h"

int main(int argc, char const *argv[])
{
    int port = 0;
    char *address;
    bool server = false;
    if (argc > 1)
    {
        if (strcmp(argv[1], "-l") == 0)
        {
            server = true;
            port = atoi(argv[2]);
            start_server(port);
        }
        else if (strcmp(argv[1], "-c") == 0 || argc >= 3)
        {
            address = malloc(strlen(argv[2]) + 1);
            strcpy(address, argv[2]);
            port = atoi(argv[3]);
            start_client(address, port);
            free(address);
        }
        else
        {
            printf("Error: could not parse arguments");
            return -1;
        }
    }

    
    return 0;
}
