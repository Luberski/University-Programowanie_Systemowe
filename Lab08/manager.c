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

int words = 20577922;
char* main_hash;
char* salt;

typedef struct msgbuf {
	long mtype;
	int id;
	int data;
	int results;
	int pass;
	int start;
	int stop;
	char hash[256];
	char salt[16];
} message_buf;


void extractor(char* hash) {
	int $sign = 0;
	char* tmp_salt = malloc(16 * sizeof(char));
	char* tmp_hash = malloc(256 * sizeof(char));
	int i = 0;
	int j = 0;
	while(hash[i] != '\0') {
		if($sign < 3) {
			if(hash[i] == '$') {
				$sign += 1;
			}
			
			tmp_salt[i] = hash[i];
		}
		else {
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
	char* dict;
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

// // read dictionary file
// 	const char *filepath = dvalue;
//     int fd = open(filepath, O_RDONLY, (mode_t)0600);
//     if (fd == -1)
//     {
//         perror("Error opening file for writing");
//         exit(EXIT_FAILURE);
//     }        
//     struct stat fileInfo = {0}; 
//     if (fstat(fd, &fileInfo) == -1)
//     {
//         perror("Error getting the file size");
//         exit(EXIT_FAILURE);
//     }   
//     if (fileInfo.st_size == 0)
//     {
//         fprintf(stderr, "Error: File is empty, nothing to do\n");
//         exit(EXIT_FAILURE);
//     }

// // map dictionary file     
//     char* map = mmap(0, fileInfo.st_size, PROT_READ, MAP_SHARED, fd, 0);
//     if (map == MAP_FAILED)
//     {
//         close(fd);
//         perror("Error mmapping the file");
//         exit(EXIT_FAILURE);
//     }
   

// //shared memory

// 	int dshid = shmget(0, fileInfo.st_size, 0600 | IPC_CREAT);
// 	char* data = shmat(dshid, NULL, 0);
// 	memcpy(data,map,fileInfo.st_size);

// 	int rshid = shmget(1, tvalue+2, 0600 | IPC_CREAT);
// 	int* results = shmat(rshid, NULL, 0);
// 	int* res = malloc(16 * sizeof(int));
// 	res[0]=1;
// 	memcpy(results,res,16);

// 	int pshid = shmget(2, 16, 0600 | IPC_CREAT);
// 	char* pass = shmat(pshid, NULL, 0);
// 	char* pas = malloc(16 * sizeof(char));
// 	memcpy(pass,pas,16);

// //queue

// 	message_buf mbuf;
// 	size_t buf_length = sizeof(mbuf);
// 	int msqid = msgget(0, IPC_CREAT | 0666);

// 	printf("The task was divided into %d parts\nThe queue id is %d\n",tvalue,msqid);	

// 	int chunk_size = fileInfo.st_size/tvalue;

// 	for(int i=0; i<tvalue; i++){
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
// 	for(int i=0;i<tvalue;i++)
// 		total += results[i+1];

// 	printf("The password was found in the dictionary :)\nThe password is %s\n%d of %d (%d%%) entries from the dictionary were checked\n",pass,total,words,total*100/words);


// // unmapping dictionary file and close it
//     if (munmap(map, fileInfo.st_size) == -1)
//     {
//         close(fd);
//         perror("Error un-mmapping the file");
//         exit(EXIT_FAILURE);
//     }
//     close(fd);

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
