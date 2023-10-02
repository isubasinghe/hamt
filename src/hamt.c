#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <hamt.h>
#include <immintrin.h>

// ASSUMING MACHINE IS 64 bits
// IF NOT: SEE END OF FILE

typedef struct {
  struct hamt_node_t **vals; // 64 * 64 bits -> 4096 bits
  size_t valids;
} __attribute__((aligned(4))) hamt_node_t;

static void nodescopy(hamt_node_t *from, hamt_node_t *to) {
  to->valids = from->valids;
  for(unsigned int i=0; i < 32; i++) {
    __m128i v = _mm_loadu_si128((__m128i *)(from->vals + i));
    _mm_storeu_si128((__m128i *)(to->vals + i), v);
  }
}

hamt_node_t *create_node() {
  hamt_node_t *node = aligned_alloc(4, sizeof(hamt_node_t));
  node->valids = 0;
  node->vals = NULL;
}


hamt_node_t *hamt_insert(hamt_node_t *from, uintptr_t key, void *value) {
  hamt_node_t *node = create_node();
  nodescopy(from, node);
  return node;
}

hamt_node_t *hamt_delete(uintptr_t key) {
  hamt_node_t *node = create_node();
  return node;
}

void free_node(hamt_node_t *) {
}



// END OF FILE: FUCK YOU, MACHINE IS 64 BITS
