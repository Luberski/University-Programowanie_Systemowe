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
#include <time.h>
#include <semaphore.h>


#define MAX_PASSWORD_LENGTH 32
#define LENGTH_OF_FILE 20577922
#define CHECKED_THRESHOLD 1000
#define SIZE_OF_FIRST_1000_PASSWORDS 8909
#define SIZE 1024
#define SHM_NAME "/sharedmemory"
#define SEM_NAME "/semops"
#define showAttr(attr) printf("getattr-> mq_curmsgs: %ld, mq_msgsize: %ld, mq_maxmsg: %ld\n", attr.mq_curmsgs, attr.mq_msgsize, attr.mq_maxmsg);
#define TIME_TO_SHOW 5.0f

//srand(time_t(0));

char* salt = NULL;
char* buff = NULL;
char* password = NULL;
char* crackedPassword = NULL;
char* messageQueueName = NULL;

mqd_t messageQueue;
size_t size = 0;
int retShm;
sem_t* sem;

unsigned char isLooping = 1;

struct MessageRequest{
    char* salt;
    char* password;
    off_t start;
    off_t end;
};


char * createQueueName(){
    char* queueName = malloc(8*sizeof(char));
    queueName[0] = '/';
    queueName[1] = 'm';
    queueName[2] = 'q';
    queueName[3] = '_';


    srand((unsigned) time(0));
    for(int i = 4; i < 7; i++){
        queueName[i] = rand() % 10 + 48;
    }
    
    queueName[7] = '\0';
    return queueName;
}

char * getSalt(char * hashed){
    char * temp = NULL;
	int counter = 0;
	int index = -1;
	for(int i = 0; i < strlen(hashed); i++){
		if(hashed[i] == '$')
			counter = counter + 1;
		if(counter == 3){
			index = i;
			break;	
		}
    
	}
	if(index != -1){
	    temp = malloc((index + 1)* sizeof(char));
		memcpy(temp, hashed, index);
        temp[index] = 0;
    }
	else{
	free(temp);
	temp = NULL;
	}
	
	return temp;
}

off_t* getOffsets(char* startAdress, off_t sizeOfFile, int countOfTasks){

    off_t packetSize = sizeOfFile / countOfTasks;
    off_t* offsetOfAreas = malloc(countOfTasks * sizeof(off_t));

    offsetOfAreas[countOfTasks-1] = sizeOfFile;
    for(int i = 0; i < countOfTasks-1; i++){
        off_t offset = (i+1) * packetSize;
        while(1){
            if(*(startAdress + offset) == '\n'){
                offsetOfAreas[i] = offset;
                break;
            }
            
            offset = offset + 1;
        }

    }
    return offsetOfAreas;
}

struct MessageRequest* allocRequest(int n, off_t size){
    off_t* offsetOfAreas = getOffsets(buff, size, n);


    salt = getSalt(password);
    off_t X_start = 0;
    struct MessageRequest* messageRequest = malloc(n * sizeof(struct MessageRequest));
    for(int i = 0; i < n; i++){
        messageRequest[i].salt = salt;
        messageRequest[i].password = password;
        messageRequest[i].start =  X_start;
        messageRequest[i].end = offsetOfAreas[i];
        X_start = offsetOfAreas[i] + 1;
    }

    free(offsetOfAreas);
    return messageRequest;
}


void sigint_handler(int signum){

	mq_close(messageQueue);
    mq_unlink(messageQueueName);
    free(messageQueueName);
    buff[size+1] = '3';
    munmap(buff, size);

    retShm = shm_unlink(SHM_NAME);
    sem_close(sem);
    printf("shm_unlink [%d] -> %s\n ",retShm, SHM_NAME);
	printf("Ended\n");
    exit(0);
}

void sigusr1_handler(int sig){
    isLooping = 0;
    
}


char* generateMessage(struct MessageRequest* address, off_t sizeOfMemory){
    char* temp = malloc(1024 * sizeof(char));
    int index = 0;
    memcpy(temp, salt, strlen(salt));
    index = index + strlen(salt);

    temp[index] = ' ';
    index = index + 1;

    memcpy(temp+index, password, strlen(password));
    index = index + strlen(password);

    temp[index] = ' ';
    index = index + 1;

    int length = snprintf(NULL, 0, "%ld", sizeOfMemory);
    char * number_temp = malloc(length + 1);
    snprintf(number_temp, length+1, "%ld", sizeOfMemory);
    memcpy(temp+index, number_temp, strlen(number_temp));
    index = index + strlen(number_temp);
    free(number_temp);

    temp[index] = ' ';
    index = index + 1;

    length = snprintf(NULL, 0, "%ld", address->start);
    number_temp = malloc(length + 1);
    snprintf(number_temp, length+1, "%ld", address->start);
    memcpy(temp+index, number_temp, strlen(number_temp));
    index = index + strlen(number_temp);
    free(number_temp);

    temp[index] = ' ';
    index = index + 1;

    length = snprintf(NULL, 0, "%ld", address->end);
    number_temp = malloc(length + 1);
    snprintf(number_temp, length+1, "%ld", address->end);
    memcpy(temp+index, number_temp, strlen(number_temp));
    index = index + strlen(number_temp);
    free(number_temp);

    return temp;
}


int main(int argc, char** argv){

    int ret;
	int file;
    struct stat fileStatistic;
    opterr = 0;
    int n = 1;

    
    struct timespec timeMain;
    struct timespec timeStart;
    struct timespec timeEnd;

    if(argc == 1){
        return 0;
    }
    while((ret = getopt(argc, argv, "f:p:n:")) != -1){
        switch(ret){
                case 'f': 
                    if(argc == 3){
                        retShm = shm_unlink(SHM_NAME);
                        printf("shm_unlink [%d] -> %s\n ",retShm, SHM_NAME);
                        return 0;
                    }

                    stat(optarg, &fileStatistic);
                    
                    size_t page = sysconf(_SC_PAGESIZE);
                    //printf("%ld \n", page);
                    if(fileStatistic.st_size % page)
                        size = fileStatistic.st_size + page - (fileStatistic.st_size % page);
                    else
                        size = fileStatistic.st_size;
                    


                    file = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                    //printf("-> %d\n", file);


                    //printf("%ld -> %ld \n", size, size+page);
                    ftruncate(file, size+page);
                    if((buff = mmap(NULL, size+page, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0)) == NULL){
		                 close(file);
                        return 2;
                    }

		            close(file);
                    

                    file = open(optarg, O_RDONLY);
                    read(file, (void*)buff, fileStatistic.st_size);
                    close(file);



                    break;
                case 'p': 
                    password = optarg;
                    //printf("%s\n", password);
                    break;
                case 'n': 
                    n = atoi(optarg);
                    if(n < 1) n = 1;
                    break;

                default: abort();
        }

    }


	signal(SIGINT, sigint_handler);



    struct MessageRequest* messageRequest = allocRequest(n, fileStatistic.st_size);
    messageQueueName = createQueueName();
    struct mq_attr attr;
    //attr.mq_msgsize = sizeof(struct MessageRequest);
    //attr.mq_maxmsg = 10;
    //messageQueue = mq_open(messageQueueName, O_RDWR | O_CREAT | O_NONBLOCK, 0666, &attr);
    messageQueue = mq_open(messageQueueName, O_RDWR | O_CREAT | O_NONBLOCK, 0666, NULL);
    mq_getattr(messageQueue, &attr); //showAttr(attr);
    printf("The name of message queue -> \"%s\"\n", messageQueueName);
    printf("The id of message queue   -> %d\n", messageQueue);

    int message = 0;
    buff[size+1] = '1';
    char* msg_ptr = NULL;
    mq_getattr(messageQueue, &attr);

    
    sem= sem_open(SEM_NAME, O_RDWR | O_CREAT, 0600, 1);


    for(int idx = 0; idx < 3; idx++)
        buff[size+3+idx] = 0;
    
    clock_gettime(CLOCK_REALTIME, &timeMain);
    clock_gettime(CLOCK_REALTIME, &timeStart);
    do{
        if(attr.mq_curmsgs < attr.mq_maxmsg && message < n){      
            msg_ptr = generateMessage((messageRequest + message), size);
            //printf("%ld -> %ld\n", (messageRequest + message)->start, (messageRequest + message)->end);
            mq_send(messageQueue, msg_ptr, strlen(msg_ptr) + 1,1);
            free(msg_ptr);
            mq_getattr(messageQueue, &attr);
            //showAttr(attr);
            message = message + 1;
        }
        mq_getattr(messageQueue, &attr);

        
        clock_gettime(CLOCK_REALTIME, &timeEnd);
        if(((timeEnd.tv_sec - timeStart.tv_sec) + (timeEnd.tv_nsec - timeStart.tv_nsec) * 1e-9) > TIME_TO_SHOW){
            clock_gettime(CLOCK_REALTIME, &timeStart);
            printf("\tTime total \t-> %.2fs\n", (timeEnd.tv_sec - timeMain.tv_sec) + (timeEnd.tv_nsec - timeMain.tv_nsec) * 1e-9);
            printf("Waiting in queue \t-> %ld/%ld\n", attr.mq_curmsgs, attr.mq_maxmsg);
            printf("Sended to queue \t-> %d/%d\n", message, n);
            int* tmp = malloc(sizeof(int));
            memcpy(tmp, (int*)(buff+size+3), sizeof(int));
            printf("Ended messages  \t-> %d/%d\n", *tmp, n);
            printf("In process  \t\t-> %ld\n", message - *tmp - attr.mq_curmsgs);
            free(tmp);

        }

    } while((attr.mq_curmsgs > 0) && buff[size+1] == '1');

    sem_close(sem);
    sem_unlink(SEM_NAME);
    if(buff[size+1] == '2'){
        printf("Password -> ");
        int indexPass = size+8;
        while(*(buff + indexPass) != '\0'){
            printf("%c", buff[indexPass]);
            indexPass = indexPass + 1;
        }
        printf("\n");

        printf("\tTime total \t-> %.2fs\n", (timeEnd.tv_sec - timeMain.tv_sec) + (timeEnd.tv_nsec - timeMain.tv_nsec) * 1e-9);
        printf("Sended to queue \t-> %d/%d\n", message, n);
        int* tmp = malloc(sizeof(int));
        memcpy(tmp, (int*)(buff+size+3), sizeof(int));
        printf("Ended messages\t-> %d/%d\n", *tmp, n);
        free(tmp);

        mq_getattr(messageQueue, &attr);
        int totalChecked = 0;
        int buffSize= attr.mq_msgsize;
        char* buffRef= malloc(buffSize);
        while(attr.mq_curmsgs != 0){
            mq_getattr(messageQueue, &attr);
            mq_receive(messageQueue, buffRef, buffSize, NULL);
        }


        buff[size+1] = '3';
        mq_getattr(messageQueue, &attr);
        sleep(1);
        do {
            mq_receive(messageQueue, buffRef, buffSize, NULL);
            totalChecked = totalChecked + atoi(buffRef);
            mq_getattr(messageQueue, &attr);
        } while(attr.mq_curmsgs != 0);

        printf("Total checked passwords -> %d\n", totalChecked);
        free(buffRef);

    }
    else{
        printf("Password not found! \nChecked all passwords!\n");
        
        int* tmp = malloc(sizeof(int));
        memcpy(tmp, (int*)(buff+size+3), sizeof(int));
        printf("Ended messages\t-> %d/%d\n", *tmp, n);
        free(tmp);

    }



    free(salt);
    free(messageRequest);
    mq_close(messageQueue);
    mq_unlink(messageQueueName);
    free(messageQueueName);

 	if(munmap(buff, size) == -1){

		fprintf(stderr, "Munmap error!\n");
		return -1;
     }
    

    printf("End\n");
    return 0;

}
