#include "clib/hashmap.h"
#include "clib/arena.h"
#include "clib/iter.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define FNV_OFFSET 0xcbf29ce484222325ULL
#define FNV_PRIME 0x100000001b3ULL

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

static Entry *entry_create(HashMap *map, const void *key, const void *value) {
  Entry *e = arena_alloc(map->arena, sizeof(Entry));
  if (e == NULL)
    return NULL;

  e->key = arena_alloc(map->arena, map->key_size);
  e->value = arena_alloc(map->arena, map->val_size);

  if (e->key == NULL || e->value == NULL)
    return NULL;

  memcpy(e->key, key, map->key_size);
  memcpy(e->value, value, map->val_size);

  e->next = NULL;
  return e;
}

static void copy_buckets(HashMap *map, Entry **new_buckets, size_t new_cap) {
  for (size_t i = 0; i < map->capacity; i++) {
    Entry *node = map->buckets[i];
    while (node != NULL) {
      Entry *next = node->next;
      size_t new_i = map->hash(node->key, map->key_size) % new_cap;
      node->next = new_buckets[new_i];
      new_buckets[new_i] = node;
      node = next;
    }
  }
}

static int grow_buckets(HashMap *map) {
  size_t new_cap = map->capacity << 1;
  Entry **new = calloc(new_cap, sizeof(Entry *));
  if (new == NULL)
    return -1;
  copy_buckets(map, new, new_cap);
  free(map->buckets);
  map->buckets = new;
  map->capacity = new_cap;
  return 0;
}

int hashmap_init(HashMap *map, size_t key_size, size_t val_size, Arena *arena) {
  map->buckets = calloc(16, sizeof(Entry *));
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

void hashmap_set_functions(HashMap *map, HashFn hash, CompareFn compare) {
  map->hash = hash == NULL ? default_hash : hash;
  map->compare = compare == NULL ? default_compare : compare;
}

void *hashmap_get(const HashMap *map, const void *key) {
  size_t i = map->hash(key, map->key_size) % map->capacity;
  Entry *node = map->buckets[i];

  while (node != NULL && map->compare(key, node->key, map->key_size) != 0) {
    node = node->next;
  }
  return node == NULL ? NULL : node->value;
}

int hashmap_put(HashMap *map, const void *key, const void *value) {
  if (map->count + 1 > map->capacity * HASHMAP_LOAD_FACTOR) {
    if (grow_buckets(map) != 0)
      return -1;
  }

  size_t i = map->hash(key, map->key_size) % map->capacity;
  Entry *node = map->buckets[i];
  while (node != NULL) {
    if (map->compare(key, node->key, map->key_size) == 0) {
      memcpy(node->value, value, map->val_size);
      return 0;
    }
    node = node->next;
  }

  Entry *new_entry = entry_create(map, key, value);
  if (new_entry == NULL)
    return -1;

  new_entry->next = map->buckets[i];
  map->buckets[i] = new_entry;
  map->count++;

  return 0;
}

int hashmap_remove(HashMap *map, const void *key) {
  size_t i = map->hash(key, map->key_size) % map->capacity;
  Entry *node = map->buckets[i];
  Entry *prev = NULL;

  while (node != NULL) {
    if (map->compare(key, node->key, map->key_size) == 0) {
      if (prev == NULL) {
        map->buckets[i] = node->next;
      } else {
        prev->next = node->next;
      }
      map->count--;
      return 0;
    }
    prev = node;
    node = node->next;
  }
  return -1;
}

size_t hashmap_count(const HashMap *map) { return map->count; }

void hashmap_free(HashMap *map) {
  free(map->buckets);
  map->buckets = NULL;
}

int hashmap_iter_next(Iter *iter) {
  HashMap *map = (HashMap *)iter->collection;
  Entry *cur = (Entry *)iter->state;

  if (cur != NULL && cur->next != NULL) {
    iter->state = cur->next;
    iter->current.key = cur->next->key;
    iter->current.value = cur->next->value;
    return 0;
  }

  while (iter->index < map->capacity) {
    Entry *entry = map->buckets[iter->index];
    iter->index++;

    if (entry != NULL) {
      iter->state = entry;
      iter->current.key = entry->key;
      iter->current.value = entry->value;
      return 0;
    }
  }
  return 1;
}

Iter hashmap_iter(HashMap *map) {
  return (Iter){
      .collection = map, .index = 0, .state = NULL, .next = hashmap_iter_next};
}
