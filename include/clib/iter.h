#ifndef ITER_H
#define ITER_H

#include <stddef.h>

typedef struct {
  void *key;
  void *value;
} IterResult;

typedef struct Iter {
  void *collection;
  size_t index;
  void *state;
  IterResult current;
  int (*next)(struct Iter *self); // return 0 (success), 1 (end), -1 (error)
} Iter;

#endif