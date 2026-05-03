#include "clib/heap.h"
#include "clib/iter.h"
#include "clib/vector.h"

#include <string.h>

// MIN-HEAP: Smaller value has higher priority (moves to top)
static int min_int_cmp(const void *a, const void *b) {
  return (*(int *)a < *(int *)b) ? 1 : 0;
}

static void swap(Heap *h, size_t i, size_t j) {
  size_t n = h->data.item_size;
  char *a = (char *)h->data.items + (i * n);
  char *b = (char *)h->data.items + (j * n);

  char tmp_buffer[256];
  if (n <= sizeof(tmp_buffer)) {
    memcpy(tmp_buffer, a, n);
    memcpy(a, b, n);
    memcpy(b, tmp_buffer, n);
  } else {
    while (n--) {
      char tmp = *a;
      *a++ = *b;
      *b++ = tmp;
    }
  }
}

static void sift_up(Heap *h, size_t index) {
  while (index > 0) {
    size_t parent = (index - 1) / 2;
    void *child_ptr = (char *)h->data.items + (index * h->data.item_size);
    void *parent_ptr = (char *)h->data.items + (parent * h->data.item_size);

    if (h->compare(child_ptr, parent_ptr) == 1) {
      swap(h, index, parent);
      index = parent;
    } else
      break;
  }
}

static void sift_down(Heap *h, size_t index) {
  size_t count = h->data.count;
  for (size_t i = 0; i < count; i++) {
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    size_t largest = index;

    void *largest_ptr = (char *)h->data.items + (largest * h->data.item_size);

    if (left < count) {
      void *left_ptr = (char *)h->data.items + (left * h->data.item_size);
      if (h->compare(left_ptr, largest_ptr) == 1) {
        largest = left;
        largest_ptr = left_ptr;
      }
    }

    if (right < count) {
      void *right_ptr = (char *)h->data.items + (right * h->data.item_size);
      if (h->compare(right_ptr, largest_ptr) == 1) {
        largest = right;
        largest_ptr = right_ptr;
      }
    }

    if (largest != index) {
      swap(h, index, largest);
      index = largest;
    } else
      break;
  }
}

void heap_init(Heap *h, size_t item_size, HeapCompareFn compare) {
  vector_init(&h->data, item_size);
  h->compare = compare == NULL ? min_int_cmp : compare;
}

int heap_push(Heap *h, const void *value) {
  if (vector_push(&h->data, value) != 0)
    return -1;
  size_t i = h->data.count - 1;
  sift_up(h, i);
  return 0;
}

int heap_pop(Heap *h, void *ptr) {
  if (h->data.count == 0)
    return -1;
  swap(h, 0, h->data.count - 1);
  if (vector_pop(&h->data, ptr) != 0)
    return -1;
  sift_down(h, 0);
  return 0;
}

void *heap_peek(Heap *h) { return h->data.count > 0 ? h->data.items : NULL; }

void heap_free(Heap *h) { vector_free(&h->data); }

int heap_iter_next(Iter *iter) { return vector_iter_next(iter); }

Iter heap_iter(Heap *heap) { return vector_iter(&heap->data); }
