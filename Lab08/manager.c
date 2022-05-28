#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <crypt.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

char *main_hash;
struct stat sb;
char *salt;
char *buff;

typedef struct msgbuf
{
	long mtype;
	int id;
	int data;
	int results;
	int pass;
	int start;
	int end;
	char hash[256];
	char salt[16];
} message_buf;

void set_offsets(struct Passw *offsets, int threads)
{
	int filesize = sb.st_size;
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
		while (buff[offsets[i].end] != '\n')
		{
			offsets[i].end++;
			offsets[i + 1].start++;
		}
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

	main_hash = malloc((strlen(hash) - i + 1) * sizeof(char));
	salt = malloc((strlen(tmp_salt)) * sizeof(char));

	memcpy(main_hash, hash + i - 1, strlen(hash) - i + 1);
	memcpy(salt, tmp_salt, strlen(tmp_salt));

	free(tmp_salt);
	free(tmp_hash);
}

int main(int argc, char **argv)
{
	int filesize = sb.st_size;
	int chunk = filesize / threads;
	int work_count = 0;
	char *dict;
	int opt;
	printf("\n");

	while ((opt = getopt(argc, argv, "h:d:t:")) != -1)
	{
		switch (opt)
		{
		case 'h':
			extractor(optarg);
			break;
		case 'd':
			dict = optarg;
			break;
		case 't':
			work_count = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Usage: %s [-h] [-d] [-t]\n",
					argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	int fd = open(dict, O_RDONLY);
	fstat(fd, &sb);
	if (fd == -1)
	{
		fprintf(stderr, "Error opening file\n");
		return 1;
	}
	if ((buff = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0)) == NULL)
	{
		fprintf(stderr, "Error mapping file\n");
		return 1;
	}

	// shared memory

	int dshid = shmget(0, sb.st_size, 0600 | IPC_CREAT);
	char *data = shmat(dshid, NULL, 0);
	memcpy(data, buff, sb.st_size);

	int rshid = shmget(1, work_count + 2, 0600 | IPC_CREAT);
	int *results = shmat(rshid, NULL, 0);
	int *res = malloc(16 * sizeof(int));
	res[0] = 1;
	memcpy(results, res, 16);

	int pshid = shmget(2, 16, 0600 | IPC_CREAT);
	char *pass = shmat(pshid, NULL, 0);
	char *pas = malloc(16 * sizeof(char));
	memcpy(pass, pas, 16);

	// //queue

	// 	message_buf mbuf;
	// 	size_t buf_length = sizeof(mbuf);
	// 	int msqid = msgget(0, IPC_CREAT | 0666);

	// 	printf("The task was divided into %d parts\nThe queue id is %d\n",work_count,msqid);

	// 	int chunk_size = sb.st_size/work_count;

	// 	for(int i=0; i<work_count; i++){
	// 		mbuf.mtype = 1;
	// 		mbuf.data = dshid;
	// 		mbuf.results = rshid;
	// 		mbuf.pass = pshid;
	// 		mbuf.id = i;
	// 		mbuf.start = i*chunk_size;
	// 		mbuf.stop = (i+1)*chunk_size;
	// 		memset(mbuf.hash,0,sizeof(mbuf.hash));
	// 		strcpy(mbuf.hash, my_hash);
	// 		memset(mbuf.salt,0,sizeof(mbuf.salt));
	// 		strcpy(mbuf.salt, my_salt);
	// 		printf("to send: %s %s\n",mbuf.salt,mbuf.hash);
	// 		msgsnd(msqid, &mbuf, buf_length, IPC_NOWAIT);
	// 	}

	// //wait for result
	// 	while(results[0] != 0)
	// 	{
	// 		int a=0;
	// 	}

	// 	int total=0;
	// 	for(int i=0;i<work_count;i++)
	// 		total += results[i+1];

	// 	printf("The password was found: %s\n%d of %d (%d%%) entries from the dictionary were checked\n",pass,total,words,total*100/words);

	if (munmap(buff, sb.st_size) != 0)
	{
		fprintf(stderr, "UnMapping Failed\n");
		return 1;
	}
	fd.close();

	// //free shared memory
	// 	shmdt(data);
	// 	shmdt(results);
	// 	shmdt(pass);
	// 	msgctl(msqid, IPC_RMID, NULL);
	// 	shmctl(dshid, IPC_RMID, NULL);
	// 	shmctl(rshid, IPC_RMID, NULL);
	// 	shmctl(pshid, IPC_RMID, NULL);
	free(main_hash);
	free(salt);
}
