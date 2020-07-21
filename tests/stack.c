#include <stdint.h>
#include <stdio.h>

// Macro to print the memory address and byte values of a variable
#define PRINT_ADDR(var) do {                                      \
    unsigned char *ptr = (unsigned char*)&var;                    \
                                                                  \
    printf("%-6s %p  %02x\n", #var, ptr, *ptr);                   \
                                                                  \
    for (size_t i = 1; i < sizeof(var); i++) {                    \
        printf("       %p  %02x\n", ptr + i, *(ptr + i));         \
    }                                                             \
                                                                  \
    printf("-------------------\n");                              \
} while(0)

int main(int argc, char **argv) {
    
    // Example variables of different types
    void *a = NULL;                      // pointer initialized to NULL
    char b = 1;                          // single byte char
    uint32_t c = 0x11223344;             // 32-bit integer
    void *d = (void*)0x1020304050607080; // arbitrary pointer value

    PRINT_ADDR(d);
    PRINT_ADDR(c);
    PRINT_ADDR(b);
    PRINT_ADDR(a);
    PRINT_ADDR(argv);
    PRINT_ADDR(argc);

    return 0;
}
