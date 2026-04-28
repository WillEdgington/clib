#include "arena.h"
#include <stddef.h>

typedef struct Entry {
  void *key;
  void *value;
  struct Entry *next;
} Entry;

typedef size_t (*HashFn)(const void *key, size_t key_size);
typedef int (*CompareFn)(const void *k1, const void *k2, size_t key_size);

typedef struct {
  Entry **buckets;
  size_t count;
  size_t capacity;
  size_t key_size;
  size_t val_size;
  Arena *arena;
  HashFn hash;
  CompareFn compare;
} HashMap;

int hashmap_init(HashMap *map, size_t key_size, size_t val_size, Arena *arena);
void hashmap_set_functions(HashMap *map, HashFn hash, CompareFn compare);
int hashmap_put(HashMap *map, const void *key, const void *value);
void *hashmap_get(const HashMap *map, const void *key);
int hashmap_remove(HashMap *map, const void *key);
size_t hashmap_count(const HashMap *map);
void hashmap_free(HashMap *map);
