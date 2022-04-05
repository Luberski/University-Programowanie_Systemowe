// PS IS1 323 LAB04
// Mariusz Lubowicki
// lm46581@zut.edu.pl

// W  rodzinie  funkcji wait znajdziemy  m.in. funkcje systemowe wait3 i wait4. Działają one analogicznie do, odpowiednio, funkcji wait i waitpid. Zwracają jednak dodatkowo rozszerzoną informację o zasobach zużytych przez zakończony proces potomny. Korzystają m.in. z jednej tych funkcję napisz własną wersję polecenia time. W dalszej części instrukcji o naszej wersji polecenia time będziemy mówili program, natomiast o programie, którego czas wykonania będziemy badali: program testowy. Wymagania dotyczące programu: przyjmuje jako argumenty nazwę programu testowego do wykonania (wraz z listą argumentów programu testowego, jeżeli ich wymaga); przyjmuje opcję,  której  wystąpienie spowoduje, że wyniki działania programu testowego będą wyświetlane na ekran (np. -v); domyślnie wyniki  te,  zarówno kierowane na stdoutjak i na stderr, mają byćukryte;•przyjmuje  opcję, która określi ile razy ma zostać wykonany programtestowy(bez podania opcji domyślniejedenraz);•w wyniku działania programu wyświetlona zostanie informacja o czasach wykonania programu testowego:orzeczywistym (pomiar ze "stoperem", przybliżony, od momentu uruchomienia programutestowegodo  jego  zakończenia, np.z użyciemfunkcja clock_gettime); użytkownika (ile czasu proces wykonywał się w przestrzeni użytkownika); systemowym (ile czasu proces wykonywał się w przestrzeni jądra); jeżeli program testowy będzie wywoływany więcej niż raz (podaliśmy odpowiednią opcję) to  poza  pojedynczymi wynikami pomiarów na końcu powinny pojawić się wartości średnie (real,user, system).

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int opt;
    int program_output_visible = 0;
    int times_executed = 1;
    double real_total = 0;
    double user_total = 0;
    double system_total = 0;

    while ((opt = getopt(argc, argv, "vt:")) != -1)
    {
        switch (opt)
        {
        case 'v':
            program_output_visible = 1;
            break;
        case 't':
            if(atoi(optarg) < 2) { 
                fprintf(stderr, "Parameter -t needs value from range <2:50>\n");
                exit(EXIT_FAILURE);
            }
            else {
                times_executed = atoi(optarg);
            }
            break;
        }
    }

    // int i = 0;
    // int optv = optind;
    // int arglen = argc - optv;
    // char* args[2];
    // while(optv < argc) {
    //     args[i] = argv[optv];
    //     i++;
    //     optv++;
    // }

    for(int i = 0; i < times_executed; i++) {
        pid_t pid = fork();
        struct timespec start, end;
        struct rusage usage;
        clock_gettime(CLOCK_REALTIME, &start);

        if(pid == 0) {
            if(program_output_visible) {
                execvp(argv[optind], &argv[optind]);
            }
            else {
                close(1);
                close(2);
                int h = open("/dev/null", O_WRONLY);
                int g = open("/dev/null", O_WRONLY);
                dup2(h, 1);
                dup2(g, 2);
                execvp(argv[optind], &argv[optind]);
                
            }
        }
        else {
            wait3(NULL, 0, &usage);
            clock_gettime(CLOCK_REALTIME, &end);
            double real = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1000000000.0);
            double user = usage.ru_utime.tv_sec + (usage.ru_utime.tv_usec / 1000000.0);
            double system = usage.ru_stime.tv_sec + (usage.ru_stime.tv_usec / 1000000.0);
            printf("\nreal: %f\n", real);
            printf("user: %f\n", user);
            printf("system: %f\n", system);

            real_total += real;
            user_total += user;
            system_total += system;
        }
    }

    if (times_executed > 1) {
        printf("\nMedian\n");
        printf("real: %f\n", real_total/times_executed);
        printf("user: %f\n", user_total/times_executed);
        printf("system: %f\n", system_total/times_executed);
    }

}

