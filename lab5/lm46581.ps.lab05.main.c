// PS IS1 323 LAB05
// Mariusz Lubowicki
// lm46581@zut.edu.pl

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>


int stop_calculations = 1;
int end_main = 1;
int child_num = 0;

void sig_handler(int sig, siginfo_t *info, void *d)
{

    if (sig == SIGALRM)
    {
        stop_calculations = 0;
    }
    else if (sig == SIGINT)
    {
        end_main = 0;
    }
    else if (sig == SIGCHLD)
    {
        time_t t = time(NULL);
        char *current_time = strtok(ctime(&t), "\n");
        printf("\t\t[ %d ] [ %d ] [ %s ]\n", info->si_pid, sig, current_time);
        child_num--;
    }
}

int main(int argc, char *argv[])
{
    int max_lifetime = 0;
    int interval = 0;
    int opt;
    struct sigaction act;
    sigset_t iset;

    while ((opt = getopt(argc, argv, "m:w:")) != -1)
    {
        switch (opt)
        {
        case 'm':
            max_lifetime = atoi(optarg);
            break;
        case 'w':
            interval = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-m] [-w]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    sigemptyset(&iset);
    act.sa_sigaction = sig_handler;
    act.sa_mask = iset;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGALRM, &act, NULL);

    pid_t child_pid;
    while (end_main)
    {
        srand(time(NULL));
        child_pid = fork();
        child_num++;
        if (child_pid == 0)
        {
            int random_value = (rand() % (max_lifetime - 1 + 1)) + 1;
            alarm(random_value);

            time_t t = time(NULL);
            char *current_time = strtok(ctime(&t), "\n");
            printf("[ %d ] [ %d ] [ %s ]\n", getpid(), random_value, current_time);

            int silnia = 1;
            int i = 2;
            while (stop_calculations)
            {
                silnia = silnia * i;
                i++;
            }

            return silnia;
        }
        else {
        }
        sleep(interval);
    }

    while (child_num > 0)
    {
        sleep(1);
    }

    return 0;
}