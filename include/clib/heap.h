#ifndef HEAP_H
#define HEAP_H

#include "iter.h"
#include "vector.h"

// return 0 if a is priority over b, 1 for b over a, -1 for error
typedef int (*HeapCompareFn)(const void *a, const void *b);

typedef struct {
  Vector data;
  HeapCompareFn compare;
} Heap;

// if compare is NULL, the default to int based min heap
void heap_init(Heap *h, size_t elem_size, HeapCompareFn compare);
void heap_free(Heap *h);
int heap_push(Heap *h, const void *value);
int heap_pop(Heap *h, void *ptr);
void *heap_peek(Heap *h);
int heap_iter_next(Iter *iter);
Iter heap_iter(Heap *heap);

#endif