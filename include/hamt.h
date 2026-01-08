#ifndef HAMT_H
#define HAMT_H

#include <stdint.h>

#define NUM_CHILDREN 64
#define MAX_LEVEL 8

static inline uint64_t hash64(uint64_t x) {
  x ^= x >> 33;
  x *= 0xff51afd7ed558ccdULL;
  x ^= x >> 33;
  x *= 0xc4ceb9fe1a85ec53ULL;
  x ^= x >> 33;
  return x;
}

static inline uint64_t get_index(uint64_t hash, uint64_t level) {
  // Use 6 bits per level
  // 6 bits * 10 levels = 60 bits.
  // After level 10, this will return 0 (as shift >= 64 is undefined/0 usually)
  // Logic elsewhere must handle level >= 10 (collisions)
  return (hash >> (level * 6)) & 0x3F;
}

#endif // HAMT_H
