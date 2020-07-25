#include <stdint.h>
#include <stdio.h>

int main(int argc, char **argv) {
    uintptr_t stackBottom, stackTop;
    asm volatile ("movq %%rbp, %0;" : "=m" (stackBottom));
    asm volatile ("movq %%rsp, %0;" : "=m" (stackTop));
    int foo;
    printf("bottom: %p\n", (void*)stackBottom);
    printf("top:    %p\n", (void*)stackTop);

    printf("argc:   %p\n", &argc);
    printf("argv:   %p\n", &argv);
    printf("foo:    %p\n", &foo);

    return 0;
}
