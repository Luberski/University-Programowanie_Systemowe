//PS IS1 323 LAB02
//Mariusz Lubowicki
//lm46581@zut.edu.pl

#include <pwd.h>
#include <utmp.h>
#include <stdio.h>
#include <errno.h>

void show_users() {
    struct utmp *entry;
    struct passwd *p;
    
    setutent();
    while(entry = getutent()) {
        if (entry->ut_type == USER_PROCESS) {
            if ((p = getpwnam(entry->ut_user)) == NULL) {
                perror(entry->ut_user);
                break;
            }
            printf("%s %d\n",
                    entry->ut_user,
                    (int) p->pw_uid);
        }
    }
}