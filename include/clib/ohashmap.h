#ifndef OHASHMAP_H
#define OHASHMAP_H

#include "arena.h"

#include <stdlib.h>

typedef enum { SLOT_EMPTY = 0, SLOT_OCCUPIED, SLOT_TOMBSTONE } SlotState;

typedef struct {
  void *key;
  void *value;
  SlotState state;
} OEntry;

typedef size_t (*HashFn)(const void *key, size_t key_size);
typedef int (*CompareFn)(const void *k1, const void *k2, size_t key_size);

typedef struct {
  OEntry *buckets;
  size_t count;
  size_t capacity;
  size_t key_size;
  size_t val_size;
  HashFn hash;
  CompareFn compare;
  Arena *arena;
} OHashMap;

int ohashmap_init(OHashMap *map, size_t key_size, size_t val_size,
                  Arena *arena);
void ohashmap_set_functions(OHashMap *map, HashFn hash, CompareFn compare);
int ohashmap_put(OHashMap *map, const void *key, const void *value);
void *ohashmap_get(OHashMap *map, const void *key);
int ohashmap_remove(OHashMap *map, const void *key);
size_t ohashmap_count(const OHashMap *map);
void ohashmap_free(OHashMap *map);

#endif