#include <stdio.h>
#include <stdlib.h>
#include <crypt.h>
#include <unistd.h>



int main(int argc, char *argv[])
{
    char *password;
    // char *salt;
    struct crypt_data data;
    data.initialized = 0;
    int opt;

    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            password = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-p] [-d] [-t]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // const char *password = "diodor_1960";
    const char *salt = "$6$5MfvmFOaDU$encrypted";
    char *hash = crypt_r(password, salt, &data);
    printf("Encrypted %s\n", hash);

    // // Save encrypted password to file
    // FILE *fp = fopen("/tmp/passwd", "w");
    // if (fp == NULL)
    // {
    //     printf("Error opening file!\n");
    //     exit(1);
    // }
    // fprintf(fp, "%s", hash);
    // fclose(fp);
}