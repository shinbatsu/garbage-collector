#include <stdio.h>
#include <stdlib.h>

#include "collector.h"


#define PRINT_ADDR(var) do { \
    char *ptr = (char*)&var; \
    printf("%4s %p   %2x\n", #var, ptr, (*ptr) & 0xff); \
    ptr++; \
    for (; ptr<(char*)(&var)+sizeof(var); ptr++) { \
        printf("     %p   %2x\n", ptr, *ptr & 0xff); \
    } \
    printf("-------------------\n"); \
} while(0)


void test() {
    void *foo = collector_malloc(199);
}

typedef struct {
    void *foo;
    int *bar;
} heapThings;

heapThings *newheapThings() {
    heapThings *s = collector_malloc(sizeof(heapThings));
    s->foo = collector_malloc(1234);
    return s;
}


void recursiveAllocationsFunction(int i, heapThings *p) {
    printf(">> recursive allocation function call %d\n", i);
    p->foo = collector_malloc(sizeof(heapThings) + 100);
    p->bar = collector_malloc(sizeof(int));

    newheapThings();

    if (i == 0) return;
    recursiveAllocationsFunction(i-1, (heapThings*)p->foo);
    *p->bar = 1;
}

void *bss1 = NULL;

int main(int argc, char **argv) {
    collector_init();
    bss1 = collector_malloc(321);
    static void *bss2;
    bss2 = collector_malloc(123);

    int *a = (int*)0x0f0f0f0f;
    void *b = &a;

    for (int i=0; i<100; i++)
        recursiveAllocationsFunction(10, collector_malloc(sizeof(heapThings)));

    char bar = 1;
    void **p = NULL;
 
    int *foo = (int*)collector_malloc(100*sizeof(int));
    void *blub = collector_malloc(1000);
    test();

    heapThings *s = newheapThings();
    void *blabla = collector_malloc(23);

    *(int*)bss1 = 1;
    *(int*)bss2 = 1;
    collector_exit();
}
