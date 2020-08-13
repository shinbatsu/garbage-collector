# Garbage Collector

## Table of Contents
1. [Usage](#usage)
2. [Code example](#code-example)
3. [Limitations](#limitations)
4. [What it does](#what-it-does)
   [Pointer Detection](#ptr-detection)
   4.2 [Hash Table Management](#hash-table-management)
   4.3 [Stack Scan](#stack-scan)
   4.4 [Collection Steps](#collection-steps)


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

## Limitation s
- Memory is fred if no ptr to it exists; discarded original ptrs may cause leaks.
- CPU registers are not scanned for ptrs.
- Works only on x64 architectures (stack ptr and call frame addresses are read with inline asm).

## What it does
Allocated memory addresses are tracked in a hash-table.
During garbage collection:
- The stack and data segments are scanned for ptrs.
- Reachable memory is marked; unreachable memory is freed.
- Marked slots are reset after collection.

### Ptr Detection
- Memory addresses are scanned in contiguous regions (stack, data, BSS).
- Only values within the bounds of allocated memory are considered valid ptrs.
- Gray list is used for tricolor marking to avoid recursive scanning.

### Hash Table Management
- Each allocation is stored in a hash table slot containing the address, size, and flags.
- Slots are dynamically resized when load factor exceeds 0.75.(SImilar trick you can see in vector.h)
- Tombstones mark freed slots for reuse.

### Stack Scan
- The stack top is obtained via inline assembly reading the base ptr.
- Iteration is performed over aligned memory addresses to detect potential ptrs.

### Collection Steps
1. Scan global memory and stack for ptrs.
2. Mark reachable slots and add them to the gray list.
3. Trace gray list recursively, marking all reachable memory.
4. Sweep unmarked slots and free the memory.
5. Update thresholds for next collection.
