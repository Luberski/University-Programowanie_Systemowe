// PS IS1 323 LAB03
// Mariusz Lubowicki
// lm46581@zut.edu.pl

#include <pwd.h>
#include <utmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <grp.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <unistd.h>

void (*print_groups)(char *, gid_t);

int main(int argc, char *argv[])
{
    struct utmp *entry;
    struct passwd *p;
    int opt;
    bool host_flag = false;
    bool groups_flag = false;

    while ((opt = getopt(argc, argv, "hg")) != -1)
    {
        switch (opt)
        {
        case 'h':
            host_flag = true;
            break;
        case 'g':
            groups_flag = true;
            break;
        default:
            fprintf(stderr, "Usage: %s [-h] [-g]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    void *handle = dlopen("./libtest.so", RTLD_LAZY);
    if (!handle)
    {
        printf("No function named print_groups found.\n");
        host_flag = false;
        groups_flag = false;
    }

    setutent();
    while (entry = getutent())
    {
        if (entry->ut_type == USER_PROCESS)
        {
            if ((p = getpwnam(entry->ut_user)) == NULL)
            {
                perror("error while retrieving passwd structure");
                exit(EXIT_FAILURE);
            }

            printf("%s ", entry->ut_user);

            if (host_flag)
                printf("%s ", entry->ut_host);
            if (groups_flag)
            {
                print_groups = dlsym(handle, "print_groups");
                print_groups(entry->ut_user, p->pw_gid);
                dlclose(handle);
            }

            printf("\n");
        }
    }
}