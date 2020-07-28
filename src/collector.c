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
