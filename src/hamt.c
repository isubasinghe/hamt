#include <hamt.h>
#include <immintrin.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline void bitmap_set(uint64_t *bitmap, uint64_t bit) {
  *bitmap |= (1ULL << bit);
}

static inline bool bitmap_get(uint64_t bitmap, uint64_t bit) {
  return (bitmap >> bit) & 1ULL;
}

// ASSUMING MACHINE IS 64 bits
// IF NOT: SEE END OF FILE

enum node_type { NODE_VALUE, NODE_MAP };

typedef struct hamt_node {
  union {
    struct {
      struct hamt_node **children; // - - 64 bits
      uint64_t mask;               // - - 64 bits
    } internal_node;               // - 128 bits
    struct {
      uint64_t key; // - - 64 bits
      void *value;  // - - 64 bits
    } data;         // - 128 bits
  };                // max(128, 128) := 128 bits

  enum node_type ty;                       // 8 bits
} __attribute__((aligned(8))) hamt_node_t; // figure out right alignment

hamt_node_t *__alloc_node() {
  hamt_node_t *node = aligned_alloc(8, sizeof(hamt_node_t));
  return node;
};

hamt_node_t *create_value_node(uint64_t key, void *value) {
  hamt_node_t *node = __alloc_node();
  node->data.value = value;
  node->data.key = key;
  node->ty = NODE_VALUE;
  return node;
}

hamt_node_t *__create_map_node() {
  hamt_node_t *node = __alloc_node();
  node->ty = NODE_MAP;
  // policy -> array size is popcount(mask); initially 0, but we often allocate
  // at least 1
  node->internal_node.children = aligned_alloc(8, sizeof(hamt_node_t *));
  node->internal_node.mask = 0;
  return node;
}

void free_node(hamt_node_t *node) {
  if (node->ty == NODE_MAP) {
    free(node->internal_node.children);
  }
  free(node);
}

void to_map_node(hamt_node_t *node) {
  node->ty = NODE_MAP;
  node->internal_node.children = aligned_alloc(8, sizeof(hamt_node_t *));
  node->internal_node.mask = 0;
}

// pre condition: node->ty == NODE_VALUE;
// post condition: true iff two inserts, false if one
hamt_node_t *convert_insert_value_node(hamt_node_t *node, uint64_t key,
                                       void *value, uint64_t level) {
  uint64_t original_hash = hash64(node->data.key);
  uint64_t original_index = get_index(original_hash, level);

  uint64_t hash = hash64(key);
  uint64_t insert_index = get_index(hash, level);

  hamt_node_t *original_node =
      create_value_node(node->data.key, node->data.value);
  to_map_node(node);

  bitmap_set(&(node->internal_node.mask), original_index);
  if (original_index == insert_index) {
    node->internal_node.children[0] = original_node;
    return NULL;
  }

  bitmap_set(&(node->internal_node.mask), insert_index);

  hamt_node_t *new_node = create_value_node(key, value);
  node->internal_node.children =
      realloc(node->internal_node.children, sizeof(hamt_node_t *) * 2);
  if (original_index > insert_index) {
    node->internal_node.children[0] = new_node;
    node->internal_node.children[1] = original_node;
  } else {
    node->internal_node.children[0] = original_node;
    node->internal_node.children[1] = new_node;
  }
  return new_node;
}

hamt_node_t *hamt_upsert(hamt_node_t *from, uint64_t key, void *value) {
  uint64_t level = 0;
  hamt_node_t *curr = from;
  uint64_t hash = hash64(key);

  // pre-condition: level == 0
  if (curr->ty == NODE_VALUE) {
    if (curr->data.key == key) {
      curr->data.value = value;
      return curr;
    }
    hamt_node_t *x = convert_insert_value_node(curr, key, value, 0);
    if (x != NULL) {
      return x;
    }
  }

  while (level <= 10) {
    uint64_t index = get_index(hash, level);
    uint64_t insert_index =
        __builtin_popcountll(curr->internal_node.mask & ((1ULL << index) - 1));
    
    if(bitmap_get(curr->internal_node.mask, index) == 0) {
      // have to insert
    }else {
    
    }
    // lets navigate deeper
    hamt_node_t *child = curr->internal_node.children[insert_index];
    if (child->ty == NODE_MAP) {
      curr = child;
    } else {
      hamt_node_t *x = convert_insert_value_node(child, key, value, level + 1);
      if (x != NULL) {
        return x;
      }
      // handle collision by going deeper
    }
    level++;
  }

  if (level == 11) {
    // handle this later, we cannot go deeper due to constraints imposed by
    // `get_index`.
    return NULL;
  }
  return NULL;
}

// END OF FILE: FUCK YOU, MACHINE IS 64 BITS
