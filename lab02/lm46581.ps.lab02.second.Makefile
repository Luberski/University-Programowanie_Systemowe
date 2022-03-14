outprog: main.o lib.o
        gcc -o outprog main.o lib.o

lib.o: lm46581.ps.lab02.second.lib.c
        gcc -c lm46581.ps.lab02.second.lib.c

main.o: lm46581.ps.lab02.second.main.c
        gcc -c lm46581.ps.lab02.second.main.c