#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


bool check_if_disconnected(char *message, int status)
{

    return (strcmp(message, "/q") == 0 || status <= 0);
}
