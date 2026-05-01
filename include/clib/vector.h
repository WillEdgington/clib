#ifndef VECTOR_H
#define VECTOR_H

#include "iter.h"

#include <stddef.h>

typedef struct {
  void *items;
  size_t item_size;
  size_t count;
  size_t capacity;
} Vector;

int vector_init(Vector *v, size_t item_size);
int vector_push(Vector *v, const void *item);
void *vector_get(const Vector *v, size_t index);
void vector_free(Vector *v);
int vector_pop(Vector *v, void *ptr);
int vector_iter_next(Iter *iter);
Iter vector_iter(Vector *vector);

#endif
