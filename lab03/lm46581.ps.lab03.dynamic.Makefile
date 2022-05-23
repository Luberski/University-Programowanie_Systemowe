# PS IS1 323 LAB03
# Mariusz Lubowicki
# lm46581@zut.edu.pl

output: main.o libtest.so
	gcc main.o -o output -ldl

libtest.so: lib.o
	gcc -shared -nostartfiles -fPIC lib.o -o libtest.so

lib.o: lm46581.ps.lab03.dynamic.lib.c
	gcc -c -fPIC lm46581.ps.lab03.dynamic.lib.c -o lib.o

main.o: lm46581.ps.lab03.dynamic.main.c
	gcc -c lm46581.ps.lab03.dynamic.main.c -o main.o

clean:
	rm *.o output libtest.so