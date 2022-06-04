#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <crypt.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

char *str;

int main(int argc, char **argv)
{
    int opt;
    // char *a;

    while ((opt = getopt(argc, argv, "k:t:")) != -1)
    {
        switch (opt)
        {
        case 'k':
            str = malloc((strlen(optarg))*sizeof(char));
            printf("strlen(optarg) = %ld\n", strlen(optarg));
            str = strcpy(str, optarg);
            printf("optarg address: %p\n", &optarg);
            printf("str address: %p\n", &optarg);
            // a = optarg;
            break;
        default:
            printf("Usage: %s [-k] [-t]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    // str = malloc(sizeof(char)*strlen(a));
    // strcpy(str, a);
    printf("%s\n", str);
    
    free(str);
}