// PS IS1 323 LAB03
// Mariusz Lubowicki
// lm46581@zut.edu.pl

#include <stdio.h>
#include <stdlib.h>
#include <grp.h>
#include <unistd.h>

void print_groups(char *ut_user, gid_t pw_gid)
{
    int ngroups = 0;
    getgrouplist(ut_user, pw_gid, NULL, &ngroups);
    gid_t *groups = malloc(sizeof(*groups) * ngroups);
    getgrouplist(ut_user, pw_gid, groups, &ngroups);

    printf("[");
    for (int i = 0; i < ngroups; i++)
    {
        struct group *grp = getgrgid(groups[i]);
        if (grp == NULL)
        {
            perror("getgrgid error: ");
        }

        if (i < ngroups - 1)
            printf("%s, ", grp->gr_name);
        else
            printf("%s]", grp->gr_name);
    }
    free(groups);
}