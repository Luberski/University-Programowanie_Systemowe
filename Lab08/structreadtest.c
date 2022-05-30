#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
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

struct Info
{
	char solved[64];
	int progress;
};

int main(int argc, char *argv[]) {
    int opt;
    int shm_id;

    while ((opt = getopt(argc, argv, "i:")) != -1)
	{
		switch (opt)
		{
		case 'i':
			shm_id = atoi(optarg);
			break;
		default:
			printf("Usage: %s [i]\n",
				   argv[0]);
			exit(EXIT_FAILURE);
		}
	}

    struct Info *shm_block = shmat(shm_id, NULL, 0);
    
    printf("shm_block->progress: %d\n", shm_block->progress);

}