// PS IS1 323 LAB08
// Mariusz Lubowicki
// lm46581@zut.edu.pl

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

int queue_id;
int waiting = 0;

struct Msg
{
	long mtype;
	int pass_shm_id;
	int info_shm_id;
	int filesize;
	char filename[20];
	int start;
	int end;
	char hash[256];
	char salt[16];
}; // NAJWIDOCZNIEJ NIE MOŻNA PRZEKAZYWAĆ DO KOLEJKI STRUCTA Z char *str; XD

struct Info
{
	char solved[64];
	int found;
	int progress;
};

struct Msg msg;

void* calculate(struct Msg *msg, char *shm_block, struct Info *info_shm_block)
{
    int word_start = 0;
    int err;
    for (int i = msg->start; i <= msg->end; i++) {
        // Check if queue still exists
        if(msgget(queue_id, IPC_EXCL | IPC_CREAT | 0666) != -1) { // !EEXIST
            err = errno;
            if(err == 17) {
                printf("Queue destroyed, exiting...\n");
                exit(0);
            }
        }
        if(info_shm_block->found == 1) {
            printf("The password has been found, exiting...\n");
            exit(0);
        }

        if (shm_block[i] == '\n' || shm_block[i] == '\0') {
            char* password = malloc(50 * sizeof(char));
            memcpy(password, shm_block+i-word_start, word_start);
            password[word_start] = '\0';
            
            char *hash = crypt(password, msg->salt);
            if (strcmp(hash, msg->hash) == 0) {
                strcpy(info_shm_block->solved, password);
                info_shm_block->found = 1;
            }

            info_shm_block->progress += 1;
            word_start = 0;
            free(password);
        }
        else
            word_start = word_start+1;
    }
}

void sig_handler(int sig, siginfo_t *info, void *d)
{
	if (sig == SIGTERM)
	{
        if(!waiting) {
            msgsnd(queue_id, (struct Msg *)&msg, sizeof(struct Msg), IPC_NOWAIT);
        }   
        exit(0);
	}
	else if (sig == SIGINT)
	{
		if(!waiting) {
            msgsnd(queue_id, (struct Msg *)&msg, sizeof(struct Msg), IPC_NOWAIT);
        }   
        exit(0);
	}
}

static int get_shm_blockId(char *filename, int file_size)
{
    key_t key = ftok(filename, 0);
    if (key == -1)
    {
        printf("ftok failed\n");
        return -1;
    }

    return shmget(key, file_size, IPC_CREAT | 0600);
}

int detach_shm_block(char *shm_block)
{
    if (shmdt(shm_block) == -1)
    {
        return 0;
    }

    return 1;
}

int main(int argc, char **argv)
{
    int opt;
    key_t key;
    int work_count;
    sigset_t iset;
    struct sigaction act;

    while ((opt = getopt(argc, argv, "k:t:")) != -1)
    {
        switch (opt)
        {
        case 'k':
            key = atoi(optarg);
            break;
        case 't':
            work_count = atoi(optarg);
            break;
        default:
            printf("Usage: %s [-k] [-t]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    sigemptyset(&iset);
	act.sa_sigaction = sig_handler;
	act.sa_mask = iset;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);

    queue_id = msgget(key, IPC_CREAT | 0600);
    if (queue_id == -1)
    {
        printf("Error creating queue\n");
        return 1;
    }

    // printf("Queue ID: %d\n", queue_id);

    // get message
    
    int err;
    for (int i = 0; i < work_count; i++)
    {
        waiting = 1;
        if (msgrcv(queue_id, (struct Msg*)&msg, (1000), 0, 0) == -1)
        {
            err = errno;
            if(err = 43) {
				printf("Queue removed while waiting for message, exiting...\n");
				i = work_count;
				exit(0);
			}
            printf("Error receiving message\n");
            break;
        }
        waiting = 0;

        printf("Received message: %d/%d\n", i+1, work_count);

        struct Info *info_shm_block = shmat(msg.info_shm_id, NULL, 0);
        char *shm_block = shmat(msg.pass_shm_id, NULL, 0);
        if (shm_block == (char *)-1 || shm_block == NULL)
        {
            printf("Error attaching shared memory block\n");
            return 1;
        }

        // printf("Received message: %d %d\n%s\n%s\n", msg.start, msg.end, msg.hash, msg.salt);
        calculate(&msg, shm_block, info_shm_block);

        if (shmdt(shm_block) == -1)
        {
            printf("Error while detaching shared memory\n");
            return 1;
        }
        if (shmdt(info_shm_block) == -1)
        {
            printf("Error while detaching shared memory\n");
            return 1;
        }

        printf("Finished work on message %d/%d\n\n", i+1, work_count);
    }

    return 0;
}