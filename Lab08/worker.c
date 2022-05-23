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


char* make_hash(char* password, char* salt){
	char s[200] = "$6$";
	strcat(s, salt);
	return crypt(password,s);
}


char* get_salt(char* shadow){
	char *result = malloc(9 * sizeof(char));
	int dolar = 0;
	int p = 0;
	for(int i=0; i<strlen(shadow); i++){
		if(shadow[i] == '$'){
			dolar += 1;
			continue;
		}
		if(dolar == 2){
			//printf("%c",shadow[i]);
			result[p] = shadow[i];
			p += 1;
		}
	}
	result[p] = '\0';
	return result;
}

char* get_hash(char* shadow){
	char *result = malloc(128 * sizeof(char*));
	int dolar = 0;
	int kropek = 0;
	int p = 0;
	int l = strlen(shadow);
	for(int i=0; i<l; i++){
		if(shadow[i] == '$'){
			dolar += 1;
			continue;
		}
		if(shadow[i] == ':'){
			kropek += 1;
			continue;
		}
		if(dolar == 3 && kropek <= 1){
			//printf("%c",shadow[i]);
			result[p] = shadow[i];
			p += 1;
		}
	}
	result[p] = '\0';
	return result;
}

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


int kvalue;
message_buf mbuf;


void sigint_handler(int signum, siginfo_t *siginfo, void *ucontext){
	size_t buf_length = sizeof(mbuf);
	msgsnd(kvalue, &mbuf, buf_length, IPC_NOWAIT);
	exit(EXIT_FAILURE);
}



int main(int argc, char **argv)
{

	int tvalue = 0;
	signal(SIGINT, sigint_handler);

	int opt;

	while ((opt = getopt (argc, argv, "t:k:")) != -1)
	switch (opt)
	  {
	  case 't':
		tvalue = atoi(optarg);
		break;
	  case 'k':
		kvalue = atoi(optarg);
		break;
	  default:
		return 1;
	  }


	for(int i=0; i<tvalue; i++){
	//queue
		if (msgrcv(kvalue, &mbuf, 1024, 1, 0) < 0) {
			perror("msgrcv");
			exit(1);
		}
	//shared memory
		int dshid = mbuf.data;
		char* data = shmat(dshid, NULL, 0);

		int rshid = mbuf.results;
		int* results = shmat(rshid, NULL, 0);

		int pshid = mbuf.pass;
		char* pass = shmat(pshid, NULL, 0);

	//do jobs
		printf("id:   %d\n",mbuf.id);
		printf("start:   %d\n",mbuf.start);
		printf("stop:   %d\n",mbuf.stop);
		printf("hash:   %s\n",mbuf.hash);
		printf("salt:   %s\n",mbuf.salt);

		off_t start_size = mbuf.start;
		off_t stop_size = mbuf.stop;

		char *result = malloc(64 * sizeof(char));
		int i = start_size;
		int j = 0;
		int tmp = 1;

		while(i < stop_size && results[0] != 0)
		{
			if(data[i] == 10){
				results[mbuf.id+1] += 1;
				result[j] = '\0';
				tmp = strcmp(mbuf.hash, get_hash(make_hash(result, mbuf.salt)));
				//tmp = strcmp(result,"0123");
				if(tmp == 0){
					results[0] = 0;
					strcpy(pass,result);
				}
				*result = malloc(64 * sizeof(char));
				j = 0;
			}else{
				result[j] = data[i];
				j += 1;
			}
			i += 1;
		}

	}


}
