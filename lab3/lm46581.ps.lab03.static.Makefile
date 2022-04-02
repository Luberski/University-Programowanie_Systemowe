# PS IS1 323 LAB03
# Mariusz Lubowicki
# lm46581@zut.edu.pl

outprog: main.o libtest.a
	gcc main.o -o static -L. -ltest

libtest.a: lib.o
	ar cr libtest.a lib.o

lib.o: lm46581.ps.lab03.static.lib.c
	gcc -c lm46581.ps.lab03.static.lib.c

main.o: lm46581.ps.lab03.static.main.c
	gcc -c lm46581.ps.lab03.static.main.c
