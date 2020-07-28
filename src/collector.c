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
  printf(" * find slot for %p (capacity: %d, count: %d)\n", (void *)addr,
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

static void adjustSlotsCapacity(int capacity) {
  COLLECTOR_Slot *oldSlots = collector->slots;
  int oldCapacity = collector->slotsCollection;
  int oldCount = collector->slotsNum;

  if (capacity <= oldCapacity)
    return;

  collector->slots = malloc(capacity * sizeof(COLLECTOR_Slot));
  if (collector->slots == NULL)
    exit(1);
  collector->slotsCollection = capacity;
  collector->slotsNum = 0;

#ifdef COLLECTOR_DEBUG
  printf("Adjust slots capacity from %d to %d\n", oldCapacity, capacity);
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