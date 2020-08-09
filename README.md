# Garbage Collector

**BSS** - Block Started by Symbol. Used for static or global vars.

Imlementation of a conservative stop-the-world mark-and-sweep garbage collector for C.
It tracks heap memory allocated via `collector_malloc()` and `collector_realloc()`.
It scan the stack, BSS, and data segments for ptr to determine memory that can be reache4d.

## Usage
Iniits before any allocations:

```
void collector_init();
```

Allocate memory:

```
void *collector_malloc(size_t size);
void *collector_realloc(void *ptr, size_t size);
```

Cleanup at program exit:

```
void collector_exit();
```

Compile with optional debug:
```
gcc -DCOLLECTOR_DEBUG -DCOLLECTOR_DEBUG_HASHTABLE -o test src/*.c
```

## Code example

```
#include "collector.h"

void heapThings() {
    void *p = collector_malloc(123);
}

int main() {
    collector_init();
    heapThings();
    collector_exit();
}
```
