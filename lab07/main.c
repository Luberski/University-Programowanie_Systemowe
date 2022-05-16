#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

// $6$5MfvmFOaDU$3Sn0LwzVT/dmzejrxvnDjGEX0W03YucvvDDBvX7QEACWUnwLA7MLjyOHHygMLzDhxwqYLYlry.73KOK9MMkKA1
pthread_mutex_t lock;
char *buff;
char *main_hash = "$6$5MfvmFOaDU$3Sn0LwzVT/dmzejrxvnDjGEX0W03YucvvDDBvX7QEACWUnwLA7MLjyOHHygMLzDhxwqYLYlry.73KOK9MMkKA1";
struct stat sb;
int stop_flag = 0;
float last_progress = 0;
long int progress = 0;
int speed_test_flag = 0;
int word_count = 0;

struct Passw {
    int start;
    int end;
};

void compare_hash(char* string) {
    struct crypt_data data;
    data.initialized = 0;

    char *hash = crypt_r(string, "$6$5MfvmFOaDU$encrypted", &data);
    if(last_progress+0.1 < ((float)(progress*100) / (float)sb.st_size)) {
        printf("\033[2J");
        printf("Progress: %0.1f%%\n", ((float)(progress*100) / (float)sb.st_size));
        last_progress = ((float)(progress*100) / (float)sb.st_size);
    }

    if (strcmp(hash, main_hash) == 0) {
        // printf("\033[2J");
        printf("%s\n", string);
        stop_flag = 1;
    }
}

void set_offsets(struct Passw *offsets, int threads) {
    int filesize = sb.st_size;
    int chunk = filesize / threads;
    int offset = 0;
    int i;

    for (i = 0; i < threads; i++) {
        offsets[i].start = offset;
        offsets[i].end = (i + 1) * chunk;
        offset = ((i + 1) * chunk) + 1;
    }
    offsets[threads - 1].end = filesize;

    for (i = 0; i < threads-1; i++) {
        while(buff[offsets[i].end] != '\n') {
            offsets[i].end++;
            offsets[i+1].start++;
        }
    }
}

void* thread_speed_test(void *offsets) {
    struct Passw *offset = (struct Passw *) offsets;
    int word_start = 0;
    int word_count = 0;
    for (int i = offset->start; i <= offset->end; i++) {
        if (buff[i] == '\n' ) {
            char* password = malloc(50 * sizeof(char));
            memcpy(password, buff+i-word_start, word_start);
            struct crypt_data data;
            data.initialized = 0;
            char *hash = crypt_r(password, "$6$5MfvmFOaDU$encrypted", &data);
            password[word_start] = '\0';
            word_start = 0;
            word_count++;
            free(password);
        }
        else
            word_start = word_start+1;
    }
    
    printf("Thread %ld: %d passwords searched\n", pthread_self(), word_count);
}

void* thread_func(void *offsets)
{
    struct Passw *offset = (struct Passw *) offsets;
    int word_start = 0;
    for (int i = offset->start; i <= offset->end; i++) {
        if (buff[i] == '\n' ) {
            char* password = malloc(50 * sizeof(char));
            memcpy(password, buff+i-word_start, word_start);
            compare_hash(password);
            pthread_mutex_lock(&lock);
            progress+=word_start+1;
            pthread_mutex_unlock(&lock);
            password[word_start] = '\0';
            word_start = 0;
            free(password);
        }
        else
            word_start = word_start+1;

        if (stop_flag) pthread_exit(NULL);

        
    }

    
}

int main(int argc, char *argv[])
{
    int index = argc == 1 ? 0 : atoi(argv[1]);
    char *dict;
    char *salt = "$6$5MfvmFOaDU$encrypted";
    int threads_num = -500;
    int opt;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "mutex init failed\n");
        return 1;
    }


    // struct crypt_data data;
    // data.initialized = 0;

    while ((opt = getopt(argc, argv, "h:d:t:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            
            // main_hash = malloc(strlen(optarg) * sizeof(char));
            // main_hash = optarg;
            // printf("decode: $6$5MfvmFOaDU$3Sn0LwzVT/dmzejrxvnDjGEX0W03YucvvDDBvX7QEACWUnwLA7MLjyOHHygMLzDhxwqYLYlry.73KOK9MMkKA1\n");
            // printf("optarg: %s\n", main_hash);
            // char *hash = crypt_r("aaron1972", "$6$5MfvmFOaDU$encrypted", &data);
            // printf("%s\n", hash);
            break;
        case 'd':
            dict = optarg;
            break;
        case 't':
            threads_num = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-h] [-d] [-t]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // printf("Threads: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));


    if(threads_num == -500) {
        speed_test_flag = 1;
    }
    else if (threads_num > sysconf(_SC_NPROCESSORS_ONLN) || threads_num < 1)
    {
        fprintf(stderr, "Invalid number of threads\n");
        threads_num = sysconf(_SC_NPROCESSORS_ONLN);
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


    if(speed_test_flag) {
        for(int i = 2; i <= sysconf(_SC_NPROCESSORS_ONLN); i++) {
            pthread_t threads[i];
            struct Passw offsets[i];
            sb.st_size = 10000;
            set_offsets(offsets, i);

            printf("Measuring time for %d threads\n", i);
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            for (int j = 0; j < i; j++) {
                pthread_create(&threads[j], NULL, thread_speed_test, (void *) &offsets[j]);
            }
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            clock_gettime(CLOCK_MONOTONIC, &end);
            double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
            printf("Time spent: %f s\n\n", time_spent);
        }
        

        return 0;
    }


    struct Passw *offsets = calloc(threads_num, sizeof(*offsets));
    set_offsets(offsets, threads_num);

    // print offsets
    // for (int i = 0; i < threads_num; i++) {
    //     printf("%d: %d - %d\n", i, offsets[i].start, offsets[i].end);
    // }



    pthread_t *threads = malloc(threads_num * sizeof(pthread_t));
    for (int i = 0; i < threads_num; i++)
    {
        pthread_create(&threads[i], NULL, thread_func, &offsets[i]);
    }

    printf("\033[2J");
    printf("Progress: 0.0%%\n");

    for (int i = 0; i < threads_num; i++)
    {
        pthread_join(threads[i], NULL);
    }

    if (munmap(buff, sb.st_size) != 0)
    {
        fprintf(stderr, "UnMapping Failed\n");
        return 1;
    }

    free(offsets);

    return 0;
}