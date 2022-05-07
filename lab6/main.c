#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include "timer.h"

void sig_handler(int sig, siginfo_t *info, void *d)
{
    if (sig == SIGTERM)
    {
        printf("thread: %lu terminated with time: %fms\n", pthread_self(), stop());
        pthread_exit(NULL);
    }
}

void *thread_func(void *arg)
{
    start();
    int i = 1;
    int fact = 1;
    while (1)
    {
        fact *= i;
        i++;
    }
}

int main(int argc, char *argv[])
{
    int max_lifetime = 0;
    int thread_count = 0;
    int opt;
    struct sigaction act;
    sigset_t iset;
    struct timespec start, end;

    while ((opt = getopt(argc, argv, "m:c:")) != -1)
    {
        switch (opt)
        {
        case 'm':
            max_lifetime = atoi(optarg);
            break;
        case 'c':
            thread_count = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-m] [-c]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    sigemptyset(&iset);
    act.sa_sigaction = sig_handler;
    act.sa_mask = iset;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &act, NULL);

    // Losujemy czas życia wątków
    double *thread_lifetime = malloc(thread_count * sizeof(double));
    for (int i = 0; i < thread_count; i++)
    {
        thread_lifetime[i] = (rand() % (max_lifetime - 1 + 1)) + 1;
    }

    // Tworzymy wątki
    pthread_t *threads = malloc(thread_count * sizeof(pthread_t));
    for (int i = 0; i < thread_count; i++)
    {
        pthread_create(&threads[i], NULL, thread_func, NULL);
        printf("Creating thread %ld with lifetime: %fs\n", threads[i], thread_lifetime[i]);
    }

    // Start timer
    double real = 0;
    clock_gettime(CLOCK_REALTIME, &start);
    while (real <= max_lifetime)
    {
        clock_gettime(CLOCK_REALTIME, &end);
        real = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1.0e9);
        for (int i = 0; i < thread_count; i++)
        {
            if (real > thread_lifetime[i])
            {
                pthread_kill(threads[i], SIGTERM);
            }
        }
    }

    return 0;
}