// PS IS1 323 LAB03
// Mariusz Lubowicki
// lm46581@zut.edu.pl

#include <pwd.h>
#include <utmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <grp.h>
#include <stdbool.h>
#include <unistd.h>

void print_groups(char *ut_user, gid_t pw_gid);

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
                print_groups(entry->ut_user, p->pw_gid);
            printf("\n");
        }
    }
}