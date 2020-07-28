#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stdint.h>
#include <stdlib.h>

typedef enum Flags {
  SLOT_UNUSED = 0,
  SLOT_IN_USE = 1,
  SLOT_MARKED = 2,
  SLOT_TOMBSTONE = 4
} Flags;

struct COLLECTOR_Slot_ {
  size_t size;
  Flags flags;
#ifdef COLLECTOR_DEBUG
  int id;
#endif
  uintptr_t addr;
};
typedef struct COLLECTOR_Slot_ COLLECTOR_Slot;

#define SLOTS_MAX_LOAD 0.75
#define SLOTS_INITIAL_CAPACITY 8
#define SLOTS_GROW_FACTOR 2
#define HEAP_GROW_FACTOR 2

typedef struct {
  void *stackBottom;
  uintptr_t minAddress;
  uintptr_t maxAddress;
  size_t bytesAllocated;
  size_t nextGC;
  int slotsNum;
  int slotsCollection;
  COLLECTOR_Slot *slots;
  int grayCount;
  int grayCapacity;
  COLLECTOR_Slot **grayList;
#ifdef COLLECTOR_DEBUG
  int lastId;
#endif
} COLLECTOR;

void collector_init_(void *stackBottom);

#define collector_init() \
  do { \
    uintptr_t stackBottom; \
    asm volatile("movq %%rbp, %0;" : "=m"(stackBottom)); \
    collector_init_((void *)stackBottom); \
  } while (0)

void collector_exit();
void *collector_malloc(size_t size);
void *collector_realloc(void *ptr, size_t newSize);
void collector_collect();

#endif
