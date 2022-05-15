#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>

pthread_mutex_t lock;
char *buff;
long int index = 0;
struct stat sb;

long int assign_password()
{
    long int next_index = 0;
    pthread_mutex_lock(&lock);
    if(index == st.st_size)
    {
        return -1;
    }
    while(index != '\n' || index < st.st_size)
    {
        index++;
    }
    next_index = index;
    pthread_mutex_unlock(&lock);

    

    return next_index;
}

void thread_func(void *arg)
{
    index = 0;

    while(index = assign_password())
    {
        index_end = index+1;
        while(index_end != '\n')
        {
            index_end++;
        }

        char* pass = malloc(index_end-index);
        for (int i = index; i < index_end; i++)
        {
            buff[i] = '\0';
        }
    }
}

int main(int argc, char *argv[])
{
    int index = argc == 1 ? 0 : atoi(argv[1]);
    char *hash;
    char *dict;
    char *salt = "$6$5MfvmFOaDU$encrypted";
    int threads_num = 1;
    int opt;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "mutex init failed\n");
        return 1;
    }

    while ((opt = getopt(argc, argv, "h:d:t:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            hash = optarg;
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

    if (threads_num > sysconf(_SC_NPROCESSORS_ONLN) || threads_num < 1)
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

    // pthread_t *threads = malloc(threads_num * sizeof(pthread_t));
    // for (int i = 0; i < threads; i++)
    // {
    //     pthread_create(&threads[i], NULL, thread_func, NULL);
    // }

    // for (int i = 0; i < threads; i++)
    // {
    //     pthread_join(threads[i], NULL);
    // }

    if (munmap(buff, sb.st_size) != 0)
    {
        fprintf(stderr, "UnMapping Failed\n");
        return 1;
    }

    return 0;
}