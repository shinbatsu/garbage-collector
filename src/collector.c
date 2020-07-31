#include "collector.h"
#include <stdint.h>
#ifdef COLLECTOR_DEBUG
#include <stdio.h>
#endif

COLLECTOR *collector;

static uint32_t hashAddress(uintptr_t addr) { 
  // Convert addr to 32 bit int hash
  uint32_t hash = 2166136261u; // salt
  uint8_t *a = (uint8_t *)&addr;
  hash ^= a[0];
  hash *= 16777619;
  hash ^= a[1];
  hash *= 16777619;
  hash ^= a[2];
  hash *= 16777619;
  hash ^= a[3];
  hash *= 16777619;
  return hash;
  return (13 * addr) ^ (addr >> 15);
}


static COLLECTOR_Slot *findSlot(uintptr_t addr) {
  if (collector->slotsCollection == 0)
    return NULL;
  uint32_t hash = hashAddress(addr);
  uint32_t idx = hash % collector->slotsCollection;
  COLLECTOR_Slot *tombstone = NULL;
#ifdef COLLECTOR_DEBUG_HASHTABLE
  int collisiontCount = 0;
  printf(" * find slot for %p (size: %d, count: %d)\n", (void *)addr,
         collector->slotsCollection, collector->slotsNum);
#endif

  while (1) {
    COLLECTOR_Slot *slot = &collector->slots[idx];
    if (slot->addr == 0) {
      if (slot->flags == SLOT_UNUSED) {
#ifdef COLLECTOR_DEBUG_HASHTABLE
        if (tombstone != NULL) {
          printf(" * reusing tombstone\n");
        }
#endif
        return tombstone != NULL ? tombstone : slot;
      } else {
        if (tombstone == NULL)
          tombstone = slot;
      }
    } else if (slot->addr == addr) {
      return slot;
    }
    idx = (idx + 1) % collector->slotsCollection;
#ifdef COLLECTOR_DEBUG_HASHTABLE
    printf(" * hash table collision %d\n", ++collisiontCount);
#endif
  }
}

static void adjustSlotsCapacity(int size) {
  COLLECTOR_Slot *oldSlots = collector->slots;
  int oldCapacity = collector->slotsCollection;
  int oldCount = collector->slotsNum;

  if (size <= oldCapacity)
    return;

  collector->slots = malloc(size * sizeof(COLLECTOR_Slot));
  if (collector->slots == NULL)
    exit(1);
  collector->slotsCollection = size;
  collector->slotsNum = 0;

#ifdef COLLECTOR_DEBUG
  printf("Adjust slots size from %d to %d\n", oldCapacity, size);
#endif

  /* initialize new slots */
  for (int i = 0; i < collector->slotsCollection; i++) {
    COLLECTOR_Slot *slot = &collector->slots[i];
    slot->size = 0;
    slot->flags = SLOT_UNUSED;
    slot->addr = 0;
#ifdef COLLECTOR_DEBUG
    slot->id = -1;
#endif
  }

  for (int i = 0; i < oldCapacity; i++) {
    COLLECTOR_Slot *slot = &oldSlots[i];
    if (slot->flags == SLOT_UNUSED || slot->flags & SLOT_TOMBSTONE) {
      continue;
    }
    COLLECTOR_Slot *newSlot = findSlot(slot->addr);
    newSlot->addr = slot->addr;
#ifdef COLLECTOR_DEBUG
    newSlot->id = slot->id;
#endif
    newSlot->size = slot->size;
    newSlot->flags = slot->flags;
    collector->slotsNum++;
  }

  /* free old table */
  free(oldSlots);
}


static void growSlotsCapacity() {
  int newSize = collector->slotsCollection == 0
                        ? SLOTS_INITIAL_CAPACITY
                        : collector->slotsCollection * SLOTS_GROW_FACTOR;
#ifdef COLLECTOR_DEBUG_HASHTABLE
  printf(" * grow slots size to %d\n", newSize);
#endif
  adjustSlotsCapacity(newSize);
}

static COLLECTOR_Slot *getSlot(uintptr_t addr) {
  COLLECTOR_Slot *slot = findSlot(addr);
  if (slot == NULL ||
      (slot->addr == 0 &&
       collector->slotsNum + 1 > collector->slotsCollection * SLOTS_MAX_LOAD)) {
    growSlotsCapacity();
    slot = findSlot(addr);
  }
  if (slot->addr == 0) {
    if (slot->flags ==
        SLOT_UNUSED) {
      collector->slotsNum++;
    }
    slot->addr = addr;
    slot->flags = SLOT_IN_USE;
#ifdef COLLECTOR_DEBUG
    slot->id = collector->lastId++;
#endif
  }
  return slot;
}

void markGray(COLLECTOR_Slot *slot) {
  if (collector->grayCount + 1 >= collector->grayCapacity) {
    collector->grayCapacity = collector->grayCapacity == 0
                            ? SLOTS_INITIAL_CAPACITY
                            : collector->grayCapacity * SLOTS_GROW_FACTOR;
    collector->grayList =
        realloc(collector->grayList, collector->grayCapacity * sizeof(COLLECTOR_Slot *));
  }
  collector->grayList[collector->grayCount++] = slot;
}

void collector_init_(void *stackBottom) {
  collector = malloc(sizeof(COLLECTOR));
  collector->stackBottom = stackBottom;
  collector->minAddress = UINTPTR_MAX;
  collector->maxAddress = 0;

  collector->bytesAllocated = 0;
  collector->nextGC = 1024;

  collector->slots = NULL;
  collector->slotsNum = 0;
  collector->slotsCollection = 0;

  collector->grayCount = 0;
  collector->grayCapacity = 0;
  collector->grayList = NULL;

#ifdef COLLECTOR_DEBUG
  collector->lastId = 0;

  printf("== COLLECTOR initialized\n");
#endif
}

static void freeSlotAndMemory(COLLECTOR_Slot *slot, int freeMemory) {
  collector->bytesAllocated -= slot->size;

  if (freeMemory) {
    free((void *)slot->addr);
  }

  slot->addr = 0;
  slot->flags = SLOT_TOMBSTONE;
  slot->size = 0;
}

static void freeSlot(COLLECTOR_Slot *slot) {
  freeSlotAndMemory(slot, 1);
}

void collector_exit() {
  for (int i = 0; i < collector->slotsCollection; i++) {
    COLLECTOR_Slot *slot = &collector->slots[i];
    if (slot->flags & SLOT_IN_USE)
      freeSlot(slot);
  }

  free(collector->slots);
  free(collector->grayList);
  free(collector);
}

static void collectIfNecessary() {
#ifdef COLLECTOR_STRESS
  collector_collect();
#else
  if (collector->bytesAllocated > collector->nextGC) {
    collector_collect();
  }
#endif
}
