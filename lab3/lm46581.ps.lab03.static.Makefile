# PS IS1 323 LAB03
# Mariusz Lubowicki
# lm46581@zut.edu.pl

output: main.o libtest.a
	gcc main.o -o output -L. -ltest

libtest.a: lib.o
	ar cr libtest.a lib.o

lib.o: lm46581.ps.lab03.static.lib.c
	gcc -c lm46581.ps.lab03.static.lib.c -o lib.o

main.o: lm46581.ps.lab03.static.main.c
	gcc -c lm46581.ps.lab03.static.main.c -o main.o

clean:
	rm *.o output libtest.a