// Naszym zadaniem będzie napisanie programu, który pozwoli na przetestowanie obsługi przychodzących sygnałów. Program uruchamiamy z dwoma parametrami liczbowymi:-maksymalną długością życia procesów potomnych (sekundy) oraz-przerwą pomiędzy ich tworzeniem (sekundy).Program główny w pętli, w stałych odstępach czasu (podanych jako argument wywołania), tworzy  nowyproces potomny. Proces potomny losuje liczbę z zakresu do podanego jako argument maksimum. Następnie wykonuje dowolneobliczenia (np. liczymy kolejne wartości silni) przez okres równy wylosowanej wartości (posłużyć się funkcją alarm). Procespotomny powinien obsłużyć sygnał SIGALRMi zakończyć się, zwracając jako kod powrotu wylosowaną wcześniej liczbę.Każdy proces potomny po uruchomieniu wyświetla informację o sobie w jednej kolumnie (pid, wylosowana wartość, czas  utworzenia).Główny program aktywnie kontroluje zakończenie procesów potomnych: po odebraniu informacji o zakończeniu potomkawyświetla w drugiejkolumnie   otrzymane informacja  (pid  zakończonego  procesu,  kod  zakończenia, czas zakończenia). Czekanie ma być zaimplementowane bez użycia funkcji z rodziny wait-należy wykorzystać fakt, że proces macierzysty otrzymuje sygnał SIGCHLD po zakończeniu potomka.Podpowiedź: Należy użyć funkcji sigaction, zwracającszczególną uwagę na flagę SA_SIGINFOi budowę struktury siginfo_t.Program działaw pętli tak długo, aż otrzyma sygnał SIGINT. Powinien wtedy zaczekać na zakończenie wszystkich uruchomionych  do  tej  porypotomków, nie tworząc już nowych i zakończyć swoje działanie. Zwróć uwagę, żeby SIGINTwysyłane za pomocą kombinacji klawiszyCtrl-Cnieprzerwało działania procesów potomnych (sygnał kierowany jest w takiej sytuacji do grupy procesów).

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int max_lifetime = 0;
    int interval = 0;
    int opt;

    while ((opt = getopt(argc, argv, "m:w:")) != -1)
    {
        switch (opt)
        {
        case 'm':
            max_lifetime = atoi(optarg);
            break;
        case 'w':
            interval = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-m] [-w]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    printf("max_lifetime: %d, interval: %d\n", max_lifetime, interval);
    
    // Print random value from 1 to max lifetime
    int random_value = (rand() % (max_lifetime - 1 + 1)) + 1;
    printf("random_value: %d\n", random_value);

    struct sigaction act;
    act.sa_handler = SIG_IGN;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);

}