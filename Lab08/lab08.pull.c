// PS IS1 322 LAB08
// Sebastian Byczyk
// bs46482@zut.edu.pl


#define _GNU_SOURCE
#include <crypt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <string.h>
//#include <pthread.h>       
#include <sys/mman.h>
#include <sys/stat.h>  
#include <fcntl.h>  
#include <signal.h>
#include <semaphore.h>

#define SEM_NAME "/semops"
#define SHM_NAME "/sharedmemory"
#define MAX_PASSWORD_LENGTH 32

char* salt = NULL;
char* password = NULL;
char* crackedPassword = NULL;
off_t sizeOfFile = 0;
int lengthOfStringOfSize = 0;
off_t start;
off_t end;


#define STRUCT_INFORMATION 5

char* messageQueueName = NULL;
char* shmName = NULL;
char* buff_map = NULL;
char* buff = NULL;

mqd_t messageQueue;

unsigned char isEndedPacket = 1;

int counterOfChecked = 0;


void freeMemory(){
    free(crackedPassword);
    free(buff);
    free(salt);
    free(password);
}
mqd_t messageQueueHelper;
void sigint_handler(int signum){

    if(isEndedPacket == 0 && buff != NULL){
        printf("Waiting \n");
        int retValue = -1;
        while(retValue == -1){

            if((messageQueueHelper = mq_open(messageQueueName, O_EXCL | O_CREAT| O_NONBLOCK, 0666, NULL)) != -1){
                printf("Message queue is destroyed!\n");    
                
                
	            mq_close(messageQueueHelper);
                mq_unlink(messageQueueName);
                freeMemory();
                exit(0);
            }

            if(buff_map[sizeOfFile+1] == '1'){
                retValue = mq_send(messageQueue, buff, strlen(buff)+1,2);  
            }
            else if (buff_map[sizeOfFile+1] == '3'){
                int length = snprintf(NULL, 0, "%d", counterOfChecked);
                char * number_temp = malloc(length + 1);
                                printf("Message queue is destroyed!\n"); 
                snprintf(number_temp, length+1, "%d", counterOfChecked);
                free(number_temp);
                mq_send(messageQueue, number_temp, strlen(number_temp)+1,3);
                freeMemory();
                exit(0);

            }


        }
        printf("Sended!\n");
    }
    
    freeMemory();
    exit(0);
}

void decrypt_func(){


    int counterOfLength = 0;
    char* phrase = malloc(MAX_PASSWORD_LENGTH * sizeof(char));
    for(off_t X = start; X <= end; X++){
        if(buff_map[sizeOfFile+1] == '2'){
            printf("Password was founded!");
            freeMemory();
            return;
        }
            
        if(*(buff_map + X) == '\n'){
            counterOfChecked = counterOfChecked + 1;
            phrase[counterOfLength] = 0;
            if(strcmp(password, crypt(phrase, salt)) == 0){
                
                crackedPassword = malloc((strlen(phrase) + 1)* sizeof(char));
		        memcpy(crackedPassword, phrase, strlen(phrase));
                crackedPassword[strlen(phrase)] = '\0';
                printf("Password cracked!\n");
                printf("Password -> %s\n", crackedPassword);
                buff_map[sizeOfFile+1] = '2';
                buff_map[sizeOfFile+2] = ' ';



                buff_map[sizeOfFile+7] = ' ';
		        memcpy(buff_map + sizeOfFile+ 8, phrase, strlen(crackedPassword));
                buff_map[sizeOfFile+8+strlen(crackedPassword)] = '\0';

                int length = snprintf(NULL, 0, "%d", counterOfChecked);
                char * number_temp = malloc(length + 1);
                snprintf(number_temp, length+1, "%d", counterOfChecked);

                while(buff_map[sizeOfFile+1] == '2'){
                }
                mq_send(messageQueue, number_temp, strlen(number_temp)+1,3);
                free(number_temp);
                free(phrase);
                return;
            }
            counterOfLength = 0;

        }
        else{
            phrase[counterOfLength] = *(buff_map + X);
            counterOfLength = counterOfLength + 1;
        }
    }
    free(phrase);
    return;
}

void parserStruct(char * buffer){
    char** informationFromMessage = malloc(STRUCT_INFORMATION * sizeof(char*));
	int counter = 0;
	int counterPtr = 0;
	int index = -1;
    int startIndex = 0;
    if (salt != NULL){
        
        startIndex = (int)(strlen(salt) + strlen(password)+ lengthOfStringOfSize) + 3;
	    counterPtr = 3;

    }
	for(int i = startIndex; i < strlen(buffer); i++){
        if(buffer[i] != ' '){
            counter = counter + 1;
        }
        else{
            informationFromMessage[counterPtr] = malloc((counter + 1) * sizeof(char));
            memcpy(informationFromMessage[counterPtr], (buffer + i - counter), counter);
            counter = 0;
            counterPtr = counterPtr +1;
            if(counterPtr == STRUCT_INFORMATION - 1){
                index = i+1;
                break;
            }
        
        }
	}

    informationFromMessage[counterPtr] = malloc((strlen(buffer) - index + 1) * sizeof(char));
    memcpy(informationFromMessage[counterPtr], (buffer + index), strlen(buffer) - index + 1);

    if(salt == NULL){
        salt = malloc(strlen(informationFromMessage[0]) * sizeof(char));
        memcpy(salt, informationFromMessage[0], strlen(informationFromMessage[0]));
    }
    if(password == NULL){
        password = malloc(strlen(informationFromMessage[1]) * sizeof(char));
        memcpy(password, informationFromMessage[1], strlen(informationFromMessage[1]));
    }

    if(sizeOfFile == 0){
        sizeOfFile = atoi(informationFromMessage[2]);
        lengthOfStringOfSize = strlen(informationFromMessage[2]);
        int file = shm_open(SHM_NAME, O_RDWR , S_IRUSR);
        
        size_t page = sysconf(_SC_PAGESIZE);
        //printf("%ld -> %ld\n", sizeOfFile, page+sizeOfFile);
        ftruncate(file, sizeOfFile+page);
        buff_map = mmap(NULL, sizeOfFile+page, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
        close(file);
    }
    start = atoi(informationFromMessage[3]);
    end = atoi(informationFromMessage[4]);

    int startIndexLoop = 0;
    if(startIndex != 0)
        startIndexLoop = 3;
    for(int i = startIndexLoop; i < STRUCT_INFORMATION; i++)
        free(informationFromMessage[i]);
    free(informationFromMessage);
}



int main(int argc, char** argv){

    int ret;
    int n = 1;
    int prior = 0;
    struct mq_attr attr;
    while((ret = getopt(argc, argv, "m:n:")) != -1){
            switch(ret){
                    case 'm': 
                        messageQueueName = optarg;
                        break;
                    case 'n': 
                        n = atoi(optarg);
                        if(n < 1) n = 1;
                        break;

                    default: abort();
            }

        }

    
	signal(SIGINT, sigint_handler);

    messageQueue = mq_open(messageQueueName, O_RDWR| O_NONBLOCK, 0666, NULL);
    if(messageQueue == -1){
        printf("Message Queue doesnt exits!\n");
        return -1;
    }
    mq_getattr(messageQueue, &attr);

    sem_t*sem= sem_open(SEM_NAME, O_RDWR);

    int* tmp = malloc(sizeof(int));
    int buffSize= attr.mq_msgsize;
    buff= malloc(buffSize);
    for(int i = 0; i < n; i++){
        if(-1 != mq_receive(messageQueue, buff, buffSize, &prior)) {
            isEndedPacket = 0;
            parserStruct(buff);
            printf("%ld -> %ld | (%d)\n", start, end, prior);
            decrypt_func();
            isEndedPacket = 1;

            sem_wait(sem);
            memcpy(tmp, (int*)(buff_map+sizeOfFile+3), sizeof(int));
            *tmp = *tmp + 1;
            memcpy((buff_map+sizeOfFile+3), (char*)tmp ,  sizeof(int));
            sem_post(sem);
            //sem_close(sem);

            if(buff_map[sizeOfFile+1] != '1'){
                printf("Password was found!\n");
                free(tmp);
                freeMemory();
                return 0;

            }
        }
        else
            printf("Can't get the message!\n");
    }
    free(tmp);
    freeMemory();
    return 0;
}