#include "../include/clib/vector.h"

#include <stdlib.h>
#include <string.h>

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
  if (v->count == v->capacity && grow(v) != 0)
    return -1;
  v->count++;
  void *ptr = vector_get(v, v->count - 1);
  if (ptr == NULL) {
    v->count--;
    return -1;
  }
  memcpy(ptr, item, v->item_size);
  return 0;
}

void *vector_get(const Vector *v, size_t index) {
  if (v->count == 0 || index >= v->count)
    return NULL;
  // use char as a reference to a single byte
  return (char *)v->items + (index * v->item_size);
}

void vector_free(Vector *v) {
  free(v->items);
  v->items = NULL;
}

int vector_pop(Vector *v, void *ptr) {
  if (v->count == 0)
    return -1;
  void *tmp = vector_get(v, v->count - 1);
  v->count--;
  if (tmp == NULL)
    return -1;
  memcpy(ptr, tmp, v->item_size);
  return 0;
}