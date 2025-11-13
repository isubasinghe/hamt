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
  node->children = malloc(sizeof(hamt_node_t *)*64);
  memset(node->children, 0, sizeof(hamt_node_t *)*64);
  node->key = 0;
  return node;
}

void free_node(hamt_node_t *node) {
  free(node->children);
  free(node);
}

hamt_node_t *find_node(hamt_node_t *root, uintptr_t key, char *found, uint64_t root_level) {
  uint64_t level = root_level;
  hamt_node_t *curr = root;
  while(1) {
    uint64_t hash = hash64(key, level);
    uint64_t index = get_index(hash, level);
    if(curr->children[index] == NULL) {
      return NULL;
    }else if(curr->children[index]->value == NULL) {
      curr = curr->children[index];
      level++;
      continue;
    }

    if(curr->children[index]->key == key) {
      return curr->children[index];
    }else {
      return NULL;
    }
  }
  return NULL;
}

hamt_node_t *hamt_upsert(hamt_node_t *from, uintptr_t key, void *value, uint64_t root_level) {
  uint64_t level = root_level;
  hamt_node_t *curr = from;
  while(1) {
    uint64_t hash = hash64(key, level);
    uint64_t index =  get_index(hash, level);
    if(curr->children[index] == NULL) {
      hamt_node_t *node = create_node(value); 
      node->key = key;
      curr->children[index] = node;
      return node;
    }else if(curr->children[index]->value == NULL) {
      curr = curr->children[index];
    } else if(curr->children[index]->key == key) {
      curr->children[index]->value = value;
      return curr->children[index];
    }else {
      // collision case
      // handle push down
      hamt_node_t *internal_node = create_node(NULL);

      hamt_upsert(internal_node, curr->children[index]->key, curr->children[index]->value, level+1);
      hamt_upsert(internal_node, key, value, level + 1);

      free_node(curr->children[index]);

      curr->children[index] = internal_node;
      // mark value and key NULL so that we can distinguish leaf and internal nodes
      curr->children[index]->value = NULL;
      curr->children[index]->key = 0;
      return internal_node;
    }
    level++;
  }
  return NULL;
}


// END OF FILE: FUCK YOU, MACHINE IS 64 BITS
