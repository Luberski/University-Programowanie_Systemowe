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

char *HASH;
char *FULL_HASH;
int FILESIZE;
char *SALT;
char *BUFF;
char *FILENAME;
int queue_id;

extern int errno;

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
};

struct Info
{
	char solved[64];
	int found;
	int progress;
};

// Fix
void set_offsets(struct Msg *offsets, int threads, int filesize)
{
	int chunk = filesize / threads;
	int offset = 0;
	int i;

	for (i = 0; i < threads; i++)
	{
		offsets[i].start = offset;
		offsets[i].end = (i + 1) * chunk;
		offset = ((i + 1) * chunk) + 1;
	}
	offsets[threads - 1].end = filesize;

	for (i = 0; i < threads - 1; i++)
	{
		while (BUFF[offsets[i].end] != '\n')
		{
			offsets[i].end++;
			offsets[i + 1].start++;
		}
	}
}

static int get_shm_blockId(char *filename, int file_size)
{
	key_t key = ftok(filename, 0);
	if (key == -1)
	{
		printf("ftok failed\n");
		exit(-1);
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

int destroy_shm_block(int shm_id)
{
	if (shmctl(shm_id, IPC_RMID, NULL) == -1)
	{
		return 0;
	}

	return 1;
}

int destroy_msg_queue(int msg_id)
{
	if (msgctl(msg_id, IPC_RMID, NULL) == -1)
	{
		return 0;
	}

	return 1;
}

void sig_handler(int sig, siginfo_t *info, void *d)
{
	if (sig == SIGTERM)
	{
		if (munmap(BUFF, FILESIZE) != 0)
		{
			printf("UnMapping Failed\n");
			exit(-1);
		}
		if (!destroy_msg_queue(queue_id))
		{
			printf("Error destroying queue\n");
			exit(-1);
		}
		exit(0);
	}
	else if (sig == SIGINT)
	{
		if (munmap(BUFF, FILESIZE) != 0)
		{
			printf("UnMapping Failed\n");
			exit(-1);
		}
		if (!destroy_msg_queue(queue_id))
		{
			printf("Error destroying queue\n");
			exit(-1);
		}
		exit(0);
	}
}

void extractor(char *hash)
{
	int $sign = 0;
	char *tmp_salt = malloc(16 * sizeof(char));
	char *tmp_hash = malloc(256 * sizeof(char));
	int i = 0;
	int j = 0;
	while (hash[i] != '\0')
	{
		if ($sign < 3)
		{
			if (hash[i] == '$')
			{
				$sign += 1;
			}

			tmp_salt[i] = hash[i];
		}
		else
		{
			i += 1;
			break;
		}

		i += 1;
	}

	tmp_salt[i] = '\0';

	HASH = malloc((strlen(hash) - i + 1) * sizeof(char));
	SALT = malloc((strlen(tmp_salt)) * sizeof(char));

	memcpy(HASH, hash + i - 1, strlen(hash) - i + 1);
	memcpy(SALT, tmp_salt, strlen(tmp_salt));

	free(tmp_salt);
	free(tmp_hash);
}

int main(int argc, char **argv)
{
	struct stat sb;
	struct msqid_ds queue_data;
	struct msqid_ds ds;
	int work_count = 0;
	int curr_msg;
	int chunk = 0;
	int opt;
	struct sigaction act;
	sigset_t iset;

	while ((opt = getopt(argc, argv, "h:d:t:")) != -1)
	{
		switch (opt)
		{
		case 'h':
			FULL_HASH = malloc(strlen(optarg) * sizeof(char));
			FULL_HASH = optarg;
			// DLACZEGO ALLOC TUTAJ CHARA I POTEM FREE WYWALA PROGRAM!?!?!?!?!?!?!?!?!?!??!!??!?!?
			extractor(optarg);
			break;
		case 'd':
			FILENAME = malloc(strlen(optarg) * sizeof(char));
			FILENAME = optarg;
			break;
		case 't':
			work_count = atoi(optarg);
			curr_msg = work_count;
			chunk = FILESIZE / work_count;
			break;
		default:
			printf("Usage: %s [-h] [-d] [-t]\n",
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

	int fd = open(FILENAME, O_RDONLY);
	fstat(fd, &sb);
	FILESIZE = sb.st_size;
	if (fd == -1)
	{
		printf("Error opening file\n");
		return 1;
	}
	if ((BUFF = mmap(NULL, FILESIZE, PROT_READ, MAP_SHARED, fd, 0)) == NULL)
	{
		printf("Error mapping file\n");
		return 1;
	}

	// Pamięć współdzielona
	int shm_id = get_shm_blockId(FILENAME, FILESIZE);
	int info_shm_id = shmget(0, sizeof(struct Info), IPC_CREAT | 0600);

	if (shm_id == -1 || info_shm_id == -1)
	{
		printf("Error getting shm_id\n");
		return 1;
	}

	char *shm_block = shmat(shm_id, NULL, 0);
	struct Info *info_shm_block = shmat(info_shm_id, NULL, 0);
	info_shm_block->found = 0;
	if (shm_block == (char *)-1 || info_shm_block == (struct Info *)-1)
	{
		printf("Error mapping shm_block\n");
		return 1;
	}
	memcpy(shm_block, BUFF, FILESIZE);

	// Create offsets
	struct Msg *msg = malloc(work_count * sizeof(struct Msg));
	set_offsets(msg, work_count, FILESIZE);

	// Create queues
	key_t key = ftok(FILENAME, 0);
	printf("\nQueue key: %d\n\n", key);
	queue_id = msgget(key, IPC_CREAT | 0600);
	if (queue_id == -1)
	{
		printf("Error creating queue\n");
		return 1;
	}

	// create msg
	for (int i = 0; i < work_count; i++)
	{
		int err;
		msg[i].mtype = 1;
		msg[i].pass_shm_id = shm_id;
		msg[i].filesize = FILESIZE;
		msg[i].info_shm_id = info_shm_id;
		memset(msg[i].salt, 0, sizeof(msg[i].salt));
		memset(msg[i].hash, 0, sizeof(msg[i].hash));
		memset(msg[i].filename, 0, sizeof(msg[i].filename));
		memcpy(msg[i].filename, FILENAME, strlen(FILENAME));
		memcpy(msg[i].hash, FULL_HASH, strlen(FULL_HASH));
		memcpy(msg[i].salt, SALT, strlen(SALT));

		msg[i].salt[strlen(SALT)] = '\0';
		msg[i].hash[strlen(FULL_HASH)] = '\0';
		msg[i].filename[strlen(FILENAME)] = '\0';

		if (msgsnd(queue_id, (struct Msg *)&msg[i], sizeof(struct Msg), 0) == -1)
		{
			err = errno;
			if (err == 11)
			{
				while (err == 11) // EAGAIN
				{
					msgsnd(queue_id, (struct Msg *)&msg[i], sizeof(struct Msg), 0);
					err = errno;
				}
			}
			else
			{
				printf("errno = %d\n", err);
				printf("Error sending message\n");
				destroy_msg_queue(queue_id);
				exit(1);
			}
		}
		printf("Message %d/%d sent\n", i + 1, work_count);
	}
	// printf("\nMessages sent\n");

	// wait for enter
	while(1) {
		msgctl(queue_id, IPC_STAT, &queue_data);

		if(queue_data.msg_qnum < curr_msg) {
			curr_msg = queue_data.msg_qnum;
			printf("Pobrano wiadomosc\n");
			printf("Ilość wiadomości w kolejce: %d\n", curr_msg);
			printf("PID pobierającego: %d\n", queue_data.msg_lrpid);
		}
		if(info_shm_block->progress >= 20577920) {
			printf("Searching completed with no result\n");
			break;
		}
		else if(info_shm_block->found) {
			printf("Found password: %s\n", info_shm_block->solved);
			break;
		}

		sleep(1);
	}

	// detach & destroy
	if (!detach_shm_block(shm_block))
	{
		return -1;
	}

	if (!destroy_shm_block(shm_id))
	{
		return -1;
	}

	if (!destroy_shm_block(info_shm_id))
	{
		return -1;
	}

	if (!destroy_msg_queue(queue_id))
	{
		printf("Error destroying queue\n");
		return -1;
	}

	if (munmap(BUFF, FILESIZE) != 0)
	{
		printf("UnMapping Failed\n");
		return -1;
	}
	close(fd);

	// Free memory
	free(msg);
	free(HASH);
	free(SALT);

	return 0;
}