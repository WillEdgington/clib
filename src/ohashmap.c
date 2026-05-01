#include "clib/ohashmap.h"
#include "clib/arena.h"
#include "clib/iter.h"

#include <stddef.h>
#include <string.h>

#define FNV_OFFSET 0xcbf29ce484222325ULL
#define FNV_PRIME 0x100000001b3ULL
#define LOAD_FACTOR 0.7

// Default hash function using FNV-1a
// It treats the key as a raw sequence of bytes
static size_t default_hash(const void *key, size_t key_size) {
  size_t hash = FNV_OFFSET;
  const unsigned char *p = (const unsigned char *)key;
  for (size_t i = 0; i < key_size; i++) {
    hash ^= p[i];
    hash *= FNV_PRIME;
  }

  return hash;
}

// Default comparison function using memcmp
static int default_compare(const void *k1, const void *k2, size_t key_size) {
  return memcmp(k1, k2, key_size);
}

static void copy_buckets(OHashMap *map, OEntry *new_buckets, size_t new_cap) {
  for (size_t i = 0; i < map->capacity; i++) {
    if (map->buckets[i].state != SLOT_OCCUPIED)
      continue;

    size_t new_i = map->hash(map->buckets[i].key, map->key_size) % new_cap;
    while (new_buckets[new_i].state != SLOT_EMPTY) {
      new_i = (new_i + 1) % new_cap;
    }
    new_buckets[new_i] = map->buckets[i];
  }
}

static int grow_buckets(OHashMap *map) {
  size_t new_cap = map->capacity << 1;
  OEntry *new = calloc(new_cap, sizeof(OEntry));
  if (new == NULL)
    return -1;
  copy_buckets(map, new, new_cap);
  free(map->buckets);
  map->buckets = new;
  map->capacity = new_cap;
  return 0;
}

int ohashmap_init(OHashMap *map, size_t key_size, size_t val_size,
                  Arena *arena) {
  map->buckets = calloc(16, sizeof(OEntry));
  if (map->buckets == NULL)
    return -1;
  map->capacity = 16;
  map->count = 0;
  map->key_size = key_size;
  map->val_size = val_size;
  map->arena = arena;
  map->hash = default_hash;
  map->compare = default_compare;
  return 0;
}

void ohashmap_set_functions(OHashMap *map, HashFn hash, CompareFn compare) {
  map->hash = hash == NULL ? default_hash : hash;
  map->compare = compare == NULL ? default_compare : compare;
}

int ohashmap_put(OHashMap *map, const void *key, const void *value) {
  if (map->count + 1 > map->capacity * LOAD_FACTOR) {
    if (grow_buckets(map) != 0)
      return -1;
  }

  size_t i = map->hash(key, map->key_size) % map->capacity;
  for (size_t j; j < map->capacity; j++) {
    if (map->buckets[i].state != SLOT_OCCUPIED ||
        map->compare(key, map->buckets[i].key, map->key_size) == 0)
      break;
    i = (i + 1) % map->capacity;
  }
  if (map->buckets[i].state != SLOT_OCCUPIED) {
    if (map->buckets[i].state == SLOT_EMPTY) {
      map->buckets[i].key = arena_alloc(map->arena, map->key_size);
      map->buckets[i].value = arena_alloc(map->arena, map->val_size);
      if (map->buckets[i].key == NULL || map->buckets[i].value == NULL)
        return -1;
    }
    map->buckets[i].state = SLOT_OCCUPIED;
    map->count++;
  }
  memcpy(map->buckets[i].key, key, map->key_size);
  memcpy(map->buckets[i].value, value, map->val_size);
  return 0;
}

void *ohashmap_get(OHashMap *map, const void *key) {
  size_t i = map->hash(key, map->key_size) % map->capacity;
  for (size_t j; j < map->capacity; j++) {
    if (map->buckets[i].state == SLOT_EMPTY ||
        map->compare(key, map->buckets[i].key, map->key_size) == 0)
      break;
    i = (i + 1) % map->capacity;
  }
  return map->buckets[i].state == SLOT_OCCUPIED ? map->buckets[i].value : NULL;
}

int ohashmap_remove(OHashMap *map, const void *key) {
  size_t i = map->hash(key, map->key_size) % map->capacity;
  for (size_t j; j < map->capacity; j++) {
    if (map->buckets[i].state == SLOT_EMPTY ||
        map->compare(key, map->buckets[i].key, map->key_size) == 0)
      break;
    i = (i + 1) % map->capacity;
  }
  if (map->buckets[i].state != SLOT_OCCUPIED)
    return -1;
  map->buckets[i].state = SLOT_TOMBSTONE;
  map->count--;
  return 0;
}

size_t ohashmap_count(const OHashMap *map) { return map->count; }

void ohashmap_free(OHashMap *map) {
  free(map->buckets);
  map->buckets = NULL;
}

int ohashmap_iter_next(Iter *iter) {
  OHashMap *map = (OHashMap *)iter->collection;

  while (iter->index < map->capacity) {
    OEntry entry = map->buckets[iter->index];
    iter->index++;
    if (entry.state == SLOT_OCCUPIED) {
      iter->current.key = entry.key;
      iter->current.value = entry.value;
      return 0;
    }
  }
  return 1;
}

Iter ohashmap_iter(OHashMap *map) {
  return (Iter){.collection = map, .index = 0, .next = ohashmap_iter_next};
}
