#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <hamt.h>
#include <immintrin.h>

// ASSUMING MACHINE IS 64 bits
// IF NOT: SEE END OF FILE


typedef struct hamt_node {
  struct hamt_node **children; // 64 * 62 bits + 64 bits + 64 bits-> 4096 bits 
  uint64_t key;
  void *value;
} __attribute__((aligned(4))) hamt_node_t;

static void nodescopy(hamt_node_t *from, hamt_node_t *to) {
  for(unsigned int i=0; i < 32; i++) {
    __m128i v = _mm_loadu_si128((__m128i *)(from->children + i));
    _mm_storeu_si128((__m128i *)(to->children + i), v);
  }
}

hamt_node_t *create_node(void *value) {
  hamt_node_t *node = aligned_alloc(4, sizeof(hamt_node_t));
  node->value = value;
  node->children = malloc(sizeof(hamt_node_t)*63);
  memset(node->children, 0, sizeof(hamt_node_t)*63);
  return node;
}

hamt_node_t *find_node(hamt_node_t *root, uintptr_t key, char *found) {
  uint64_t hash = hash64(key);
  hamt_node_t *h = root;
  while(1) {
    hamt_node_t *curr = h->children[hash & 0b0000000000000000000000000000000000000000000000000000000000111111];
    if(curr->value == NULL) {
      *found = false;
      return curr;
    }
    if(key == curr->key) {
      *found = true;
      return curr;
    }
    h = curr;
  }
}

hamt_node_t *hamt_insert(hamt_node_t *from, uintptr_t key, void *value) {
  hamt_node_t *node = create_node(value);
  return node;
}


// END OF FILE: FUCK YOU, MACHINE IS 64 BITS
