#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

typedef struct Region {
  char *buffer;
  size_t capacity;
  size_t offset;
  struct Region *next;
} Region;

typedef struct {
  Region *head;
  Region *current;
  size_t slab_size;
} Arena;

int arena_init(Arena *a, size_t slab_size);
void *arena_alloc(Arena *a, size_t size);
int arena_reset(Arena *a);
void arena_free(Arena *a);

#endif
