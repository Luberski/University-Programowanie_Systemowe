// PS IS1 323 LAB09
// Mariusz Lubowicki
// lm46581@zut.edu.pl

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <syslog.h>

#define SERV_HOST_ADDR "127.0.0.1"

pthread_mutex_t lock;
char *LOG_FILE;
int isfd;
char *PORT;
char *SERVER_CATALOG;
int CLIENT;
pid_t daemon_pid = 0;
static char *saved_fname = "daemon_name.pid";
int DAEMON_ENABLED;

static int check_pid_file(const char *fname)
{
    int pid = 0;
    FILE *fp = fopen(fname, "r");

    if (fp != NULL)
    {
        if (fscanf(fp, " %d", &pid) != 1)
        {
            pid = 0;
        }

        fclose(fp);

        if (pid != 0 && pid != (int)daemon_pid)
        {
            if (kill(pid, 0) == 0)
            {
                return pid;
            }
        }
    }

    return 0;
}

int remove_pid_file(void)
{
    int rv = 0;

    if (saved_fname != NULL)
    {
        if (unlink(saved_fname) < 0)
        {
            rv = -1;
        }
    }
    return rv;
}

int write_pid_file(const char *fname)
{
    FILE *fp;
    int rv = 0;
    daemon_pid = getpid();
    fp = fopen(fname, "w");
    if (fp != NULL)
    {
        fprintf(fp, "%d\n", daemon_pid);
        fclose(fp);
    }
    else
    {
        printf("Error opening file %s\n", fname);
        rv = -1;
    }

    return rv;
}

void sig_handler(int sig, siginfo_t *info, void *d)
{
    if (sig == SIGINT)
    {
        close(isfd);
        remove_pid_file();
        exit(0);
    }
}

void *respond_to_client(void *usr_socket)
{
    char *curr_date;
    char *curr_time;
    char *client_ipaddr;
    char *response;
    char *request;
    char *status;

    response = malloc(sizeof(char) * 100);
    request = malloc(sizeof(char) * 100);
    status = malloc(sizeof(char) * 100);
    curr_date = malloc(sizeof(char) * 20);
    curr_time = malloc(sizeof(char) * 20);

    struct sockaddr_in *client_addr = (struct sockaddr_in *)usr_socket;
    client_ipaddr = inet_ntoa(client_addr->sin_addr);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(curr_date, 20, "%d/%m/%Y", &tm);
    strftime(curr_time, 20, "%H:%M:%S", &tm);

    int usr_sock = *(int *)usr_socket;
    char msgbuf[1024], *info[3], path[1024];
    int filed;

    memset((void *)msgbuf, (int)'\0', 1024);

    int err = recv(usr_sock, msgbuf, 1024, 0);
    if (err == -1)
    {
        printf("recv %d %s\n", errno, strerror(errno));
        pthread_exit((void *)0);
    }
    else if (err == 0)
    {
        printf("Client disconnected\n");
        pthread_exit((void *)0);
    }
    else
    {
        printf("Received: %s\n\n", msgbuf);
        info[0] = strtok(msgbuf, " \t\n");
        if (info[0] == NULL)
        {
            printf("Invalid request\n");
            close(usr_sock);
            pthread_exit((void *)0);
        }
        strcpy(request, info[0]);

        if (strncmp(info[0], "GET\0", strlen(info[0]) + 1) == 0)
        {
            info[1] = strtok(NULL, " \t\n\r");
            info[2] = strtok(NULL, " \t\n\r");

            if (strncmp(info[2], "HTTP/1.1\0", strlen(info[2]) + 1) == 0)
            {
                if (strncmp(info[1], "/\0", 2) == 0)
                {
                    info[1] = "/index.html";
                }

                strcpy(path, SERVER_CATALOG);
                strcat(path, info[1]);
                printf("Path: %s\n", path);
                strcpy(response, path);

                if (path[strlen(path) - 1] == '/')
                {
                    strcat(path, "index.html");
                }
                

                filed = open(path, O_RDONLY);
                if (filed != -1)
                {
                    strcpy(status, "200 OK");
                    char *buff;
                    struct stat sb;
                    fstat(filed, &sb);
                    buff = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, filed, 0);

                    write(usr_sock, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));
                    char content_length[100];
                    sprintf(content_length, "Content-Length: %ld\r\n", sb.st_size);

                    char *ext = strrchr(path, '.');
                    if (strcmp(ext, ".html") == 0)
                    {
                        write(usr_sock, "Content-Type: text/html\r\n", strlen("Content-Type: text/html\r\n"));
                    }
                    else if (strcmp(ext, ".css") == 0)
                    {
                        write(usr_sock, "Content-Type: text/css\r\n", strlen("Content-Type: text/css\r\n"));
                    }
                    else if (strcmp(ext, ".js") == 0)
                    {
                        write(usr_sock, "Content-Type: application/javascript\r\n", strlen("Content-Type: application/javascript\r\n"));
                    }
                    else if (strcmp(ext, ".jpg") == 0)
                    {
                        write(usr_sock, "Content-Type: image/jpeg\r\n", strlen("Content-Type: image/jpeg\r\n"));
                    }
                    else if (strcmp(ext, ".jpeg") == 0)
                    {
                        write(usr_sock, "Content-Type: image/jpeg\r\n", strlen("Content-Type: image/jpeg\r\n"));
                    }
                    else if (strcmp(ext, ".png") == 0)
                    {
                        write(usr_sock, "Content-Type: image/png\r\n", strlen("Content-Type: image/png\r\n"));
                    }
                    else if (strcmp(ext, ".gif") == 0)
                    {
                        write(usr_sock, "Content-Type: image/gif\r\n", strlen("Content-Type: image/gif\r\n"));
                    }
                    else if (strcmp(ext, ".ico") == 0)
                    {
                        write(usr_sock, "Content-Type: image/x-icon\r\n", strlen("Content-Type: image/x-icon\r\n"));
                    }
                    else if (strcmp(ext, ".svg") == 0)
                    {
                        write(usr_sock, "Content-Type: image/svg+xml\r\n", strlen("Content-Type: image/svg+xml\r\n"));
                    }
                    else if (strcmp(ext, ".txt") == 0)
                    {
                        write(usr_sock, "Content-Type: text/plain\r\n", strlen("Content-Type: text/plain\r\n"));
                    }
                    else if (strcmp(ext, ".pdf") == 0)
                    {
                        write(usr_sock, "Content-Type: application/pdf\r\n", strlen("Content-Type: application/pdf\r\n"));
                    }
                    else {
                        write(usr_sock, "HTTP/1.1 404 Not Found\n", 23);
                        sprintf(status, "404 Not Found");
                    }

                    write(usr_sock, content_length, strlen(content_length));
                    write(usr_sock, "Connection: close\r\n\r\n", strlen("Connection: close\r\n\r\n"));
                    write(usr_sock, buff, sb.st_size);

                    munmap(buff, sb.st_size);
                }
                else
                {
                    write(usr_sock, "HTTP/1.1 404 Not Found\n", 23);
                    sprintf(status, "404 Not Found");
                }
            }
            else
            {
                send(usr_sock, "HTTP/1.1 400 Bad Request\n", strlen("HTTP/1.1 400 Bad Request\n"), 0);
                sprintf(status, "400 Bad Request");
            }
        }
        else
        {
            send(usr_sock, "HTTP/1.1 501 Not Implemented\n", 29, 0);
            sprintf(status, "501 Not Implemented");
        }
    }

    close(usr_sock);
    close(filed);

    // Save everything to log file
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL)
    {
        printf("Error opening log file\n");
        pthread_exit((void *)0);
    }
    pthread_mutex_lock(&lock);
    fprintf(log_file, "%ld %s %s %s %s %s %s\n", syscall(__NR_gettid), curr_date, curr_time, client_ipaddr, request, response, status);
    pthread_mutex_unlock(&lock);
    fclose(log_file);

    free(curr_date);
    free(curr_time);
    free(request);
    free(response);
    free(status);

    pthread_exit((void *)0);
}

int main(int argc, char **argv)
{
    int opt;
    int work_count;
    int err;
    sigset_t iset;
    int pid;
    struct sigaction act;

    while ((opt = getopt(argc, argv, "sp:d:q")) != -1)
    {
        switch (opt)
        {
        case 's':
            if ((pid = check_pid_file(saved_fname)) > 0)
            {
                printf("Error: server is already running with pid=%d\n", pid);
                exit(1);
            }
            DAEMON_ENABLED = true;
            break;
        case 'p':
            PORT = optarg;
            break;
        case 'd':
            SERVER_CATALOG = optarg;
            struct stat sb;
            if (stat(SERVER_CATALOG, &sb) == -1)
            {
                printf("Error: catalog does not exist\n");
                exit(1);
            }
            break;
        case 'q':
            if ((pid = check_pid_file(saved_fname)) == 0)
            {
                printf("Error: server is not running\n");
                exit(1);
            }
            kill(pid, SIGINT);
            exit(1);
            break;
        case '?':
            printf("Usage: %s [-k] [-t]\n", argv[0]);
            exit(EXIT_FAILURE);
        default:
            printf("Usage: %s [-k] [-t]\n",
                   argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    if (DAEMON_ENABLED) {
        if(daemon(1,0) == -1) {
            printf("Error: daemonization failed\n");
            exit(1);
        }
    }
    
    write_pid_file(saved_fname);

    sigemptyset(&iset);
    act.sa_sigaction = sig_handler;
    act.sa_mask = iset;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, NULL);

    // Print out the configuration
    printf("Daemon: %s\n", DAEMON_ENABLED ? "Enabled" : "Disabled");
    printf("Port: %s\n", PORT);
    printf("Server Catalog: %s\n", SERVER_CATALOG);

    struct addrinfo *res, *p;

    err = getaddrinfo(SERV_HOST_ADDR, PORT, NULL, &res);
    if (err != 0)
    {
        printf("getaddrinfo failed: %s\n", gai_strerror(err));
        close(isfd);
        exit(EXIT_FAILURE);
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        isfd = socket(AF_INET, SOCK_STREAM, 0);
        int optval = 1;
        setsockopt(isfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

        if (isfd == -1)
        {
            continue;
        }

        if (bind(isfd, p->ai_addr, p->ai_addrlen) == 0)
        {
            break;
        }

        close(isfd);
    }

    freeaddrinfo(res);

    if (p == NULL)
    {
        printf("Port is occupied %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // listen
    err = listen(isfd, 5);
    if (err != 0)
    {
        printf("listen failed: %d %s\n", errno, strerror(errno));
        close(isfd);
        exit(EXIT_FAILURE);
    }

    // get program name
    // append .log to filename
    LOG_FILE = malloc(strlen(argv[0]) + 5);
    strcpy(LOG_FILE, argv[0]);
    strcat(LOG_FILE, ".log");

    // Create log file
    FILE *log_file = fopen(LOG_FILE, "w");
    if (log_file == NULL)
    {
        printf("Error opening log file\n");
        close(isfd);
        exit(EXIT_FAILURE);
    }
    fclose(log_file);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t addrlen;

        CLIENT = accept(isfd, (struct sockaddr *)&client_addr, &addrlen);
        if (CLIENT < 0)
        {
            printf("accept failed: %d %s\n", errno, strerror(errno));
            continue;
        }

        // Create a new thread for the client
        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        printf("Client: %d, %s:%d\n", CLIENT, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        err = pthread_create(&thread, &attr, respond_to_client, &CLIENT);
        if (err != 0)
        {
            printf("pthread_create failed: %d %s\n", errno, strerror(errno));
            close(isfd);
            exit(EXIT_FAILURE);
        }
    }

    close(isfd);

    return 0;
}