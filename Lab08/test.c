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

int main() {
    char str[10];
    char *cpstr = "Hello";

    memcpy(str, cpstr, sizeof(cpstr));

    printf("%s\n", str);
}