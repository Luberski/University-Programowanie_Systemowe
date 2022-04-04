#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

long long int fib(long long int n) {
    return n <= 1 ? n : fib(n - 1) + fib(n - 2);
}

int main() {
    printf("%lld\n", fib(40));
}