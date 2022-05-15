// PS IS1 323 LAB07
// Mariusz Lubowicki
// lm46581@zut.edu.pl


#include <stdio.h>
#include <stdlib.h>
#include <crypt.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char *buff;
    char *hash = "CVt7jU9wJRYz3K98EklAJqp8RMG5NvReUSVK7ctVvc2VOnYVrvyTfXaIgHn2xQS78foEJZBq2oCIqwfdNp.2V1";
    char *salt = "$6$5MfvmFOaDU$encrypted";
    char *password;
    char *pass_dict;
    int thread_count = 0;
    int opt;
    int cores = sysconf(_SC_NPROCESSORS_ONLN);

    struct crypt_data data;
    data.initialized = 0;

    // while ((opt = getopt(argc, argv, "p:d:t:")) != -1)
    while ((opt = getopt(argc, argv, "d:t:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            password = optarg;
            break;
        case 't':
            thread_count = atoi(optarg);
            break;
        case 'd':
            pass_dict = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-p] [-d] [-t]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    int fd = open("test.txt", O_RDWR);

    if(fd == -1) {
        printf("Error opening file!\n");
        exit(1);
    }
    else if(buff = mmap(NULL, 0, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) == NULL) {
        printf("Error mapping file!\n");
        exit(1);
    }

    if(thread_count > cores || thread_count < 1) {
        thread_count = cores;
    }

    pthread_t *threads = malloc(thread_count * sizeof(pthread_t));
    for (int i = 0; i < thread_count; i++)
    {
        pthread_create(&threads[i], NULL, thread_func, NULL);
        printf("Creating thread %ld with lifetime: %fs\n", threads[i], thread_lifetime[i]);
    }
    
}