#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "server.h"
#include "client.h"

int main(int argc, char const *argv[])
{
    int port = 0;
    char *adress;
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
            adress = malloc(strlen(argv[2]) + 1);
            strcpy(adress, argv[2]);
            port = atoi(argv[3]);
            start_client(adress, port);
        }
        else
        {
            printf("Error: could not parse arguments");
        }
    }

    free(adress);
    return 0;
}
