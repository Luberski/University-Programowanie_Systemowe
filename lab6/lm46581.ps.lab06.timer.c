// PS IS1 323 LAB06
// Mariusz Lubowicki
// lm46581@zut.edu.pl

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include "lm46581.ps.lab06.timer.h"

static pthread_key_t timeKey;
static pthread_once_t timeOnce = PTHREAD_ONCE_INIT;

static void createKey(void)
{
    pthread_key_create(&timeKey, NULL);
}

void start()
{
    struct timespec *time;
    
    pthread_once(&timeOnce, createKey);
    time = pthread_getspecific(timeKey);
    if (time == NULL)
    {
        time = malloc(sizeof(struct timespec));
        clock_gettime(CLOCK_REALTIME, time);
        pthread_setspecific(timeKey, time);
    }
}

double stop()
{
    struct timespec *time;
    struct timespec time_end;
    pthread_once(&timeOnce, createKey);
    time = pthread_getspecific(timeKey);
    if (time != NULL)
    {
        clock_gettime(CLOCK_REALTIME, &time_end);
    }

    return ((time_end.tv_sec - time->tv_sec) + ((time_end.tv_nsec - time->tv_nsec) / 1.0e9)) * 1000;
}