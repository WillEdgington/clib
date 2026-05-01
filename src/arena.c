#include "../include/clib/arena.h"

#include <stdlib.h>

static Region *region_create(size_t capacity) {
  Region *r = malloc(sizeof(Region) + capacity);
  if (r == NULL)
    return NULL;
  r->capacity = capacity;
  r->buffer = (char *)(r + 1); // point to memory right after struct
  r->offset = 0;
  r->next = NULL;
  return r;
}

int arena_init(Arena *a, size_t slab_size) {
  a->slab_size = slab_size;
  a->head = NULL;
  a->current = NULL;
  return 0;
}

void *arena_alloc(Arena *a, size_t size) {
  size = (size + 7) & ~7; // align requested size to 8 bytes
  if (a->head == NULL) {
    a->head = region_create(a->slab_size < size ? size : a->slab_size);
    if (a->head == NULL)
      return NULL;
    a->current = a->head;
  }

  while (a->current->capacity - a->current->offset < size) {
    if (a->current->next == NULL) {
      size_t capacity = a->slab_size < size ? size : a->slab_size;
      Region *new_r = region_create(capacity);
      if (new_r == NULL)
        return NULL;
      a->current->next = new_r;
    }
    a->current = a->current->next;
    a->current->offset = 0; // reset offset for case of old region
  }

  void *ptr = a->current->buffer + a->current->offset;
  a->current->offset += size;
  return ptr;
}

int arena_reset(Arena *a) {
  if (a->head == NULL)
    return -1;
  a->current = a->head;
  a->current->offset = 0;
  return 0;
}

void arena_free(Arena *a) {
  Region *node = a->head;
  while (node != NULL) {
    Region *next_node = node->next;
    free(node);
    node = next_node;
  }
  a->head = NULL;
  a->current = NULL;
}
