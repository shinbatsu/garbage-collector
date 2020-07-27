#ifndef SGC_H
#define SGC_H

#include <stdint.h>
#include <stdlib.h>

typedef enum Flags {
  SLOT_UNUSED = 0,
  SLOT_IN_USE = 1,
  SLOT_MARKED = 2,
  SLOT_TOMBSTONE = 4
} Flags;

struct SGC_Slot_ {
  size_t size;
  Flags flags;
#ifdef SGC_DEBUG
  int id;
#endif
  uintptr_t address;
};
typedef struct SGC_Slot_ SGC_Slot;

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
  int slotsCount;
  int slotsCapacity;
  SGC_Slot *slots;
  int grayCount;
  int grayCapacity;
  SGC_Slot **grayList;
#ifdef SGC_DEBUG
  int lastId;
#endif
} SGC;

void sgc_init_(void *stackBottom);

#define sgc_init() \
  do { \
    uintptr_t stackBottom; \
    asm volatile("movq %%rbp, %0;" : "=m"(stackBottom)); \
    sgc_init_((void *)stackBottom); \
  } while (0)

void sgc_exit();
void *sgc_malloc(size_t size);
void *sgc_realloc(void *ptr, size_t newSize);
void sgc_collect();

#endif
