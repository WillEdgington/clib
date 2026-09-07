#include "clib/vector.h"
#include "clib/iter.h"

#include <stdlib.h>
#include <string.h>

static void *get_item_unguarded(const Vector *v, size_t index) {
  // use char as a reference to a single byte
  return (char *)v->items + (index * v->item_size);
}

static int grow(Vector *v) {
  v->capacity <<= 1;
  void *tmp = realloc(v->items, v->capacity * v->item_size);
  if (tmp == NULL)
    return -1;
  v->items = tmp;
  return 0;
}

int vector_init(Vector *v, size_t item_size) {
  v->item_size = item_size;
  v->capacity = 4;
  v->count = 0;
  v->items = malloc(v->item_size * v->capacity);
  return v->items == NULL ? -1 : 0;
}

int vector_push(Vector *v, const void *item) {
  if (v == NULL || item == NULL || (v->count == v->capacity && grow(v) != 0))
    return -1;

  void *ptr = get_item_unguarded(v, v->count);

  memcpy(ptr, item, v->item_size);
  v->count++;
  return 0;
}

void *vector_get(const Vector *v, size_t index) {
  if (v == NULL || v->count == 0 || index >= v->count)
    return NULL;
  return get_item_unguarded(v, index);
}

void vector_free(Vector *v) {
  if (v == NULL) return;

  free(v->items);
  v->items = NULL;
  v->count = 0;
  v->capacity = 0;
  v->item_size = 0;
}

int vector_pop(Vector *v, void *ptr) {
  if (v == NULL || v->count == 0)
    return -1;
  
  if (ptr != NULL) {
    void *tmp =  get_item_unguarded(v, v->count - 1);
    if (tmp == NULL)
      return -1;
  
    memcpy(ptr, tmp, v->item_size);
  }

  v->count--;
  return 0;
}

int vector_remove(Vector *v, size_t index, void *ptr) {
  if (v == NULL || index >= v->count) {
    return -1;
  }

  void *item = (char *)v->items + (index * v->item_size);

  if (ptr != NULL)
    memcpy(ptr, item, v->item_size);

  size_t remaining = v->count - 1 - index;
  if (remaining > 0) {
    void *src = (char *)item + v->item_size;
    memmove(item, src, remaining);
  }
  v->count--;
  return 0;
}

int vector_iter_next(Iter *iter) {
  Vector *vector = (Vector *)iter->collection;
  if (iter->index >= vector->count) {
    iter->current.value = NULL;
    return 1;
  }
  iter->current.value =
      get_item_unguarded(vector, iter->index);
  iter->index++;
  return 0;
}

Iter vector_iter(Vector *vector) {
  return (Iter){.collection = vector, .index = 0, .next = vector_iter_next};
}
