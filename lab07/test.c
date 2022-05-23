#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>

// char* main_hash = "$6$5MfvmFOaDU$vOCCa/BJjdcLUvKQfz7Ql81tYAmaNvnLHeLkBwIm32HaI8ga25fDgW90T1W55dGoKT1f32K2eoDBHFI4NLItp/";

// void compare_hash(char* string) {
//     struct crypt_data data;
//     data.initialized = 0;

//     char *hash = crypt_r(string, "$6$5MfvmFOaDU$encrypted", &data);
//     if (strcmp(hash, main_hash) == 0) {
//         printf("%s\n", string);
//     }
// }

int main(int argc, char *argv[])
{
    if(0 < 0.1) {
        printf("True\n");
    }
}