#include "clib/heap.h"
#include "clib/metric_framework.h"
#include "clib/vector.h"

#define N_OPERATIONS 1000000

static void env_info() {
  printf("==========================================\n");
  printf("ENVIRONMENT SETUP:\n");
  printf("  Min-Heap: int\n");
  printf("==========================================\n\n");
}

static void run_basic_heap_ops() {
  Heap h;
  heap_init(&h, sizeof(int), NULL);

  BENCHMARK_FULL("Heap Push (Ascending Values)", N_OPERATIONS, {
    int val = (int)i;
    heap_push(&h, &val);
  });

  heap_free(&h);
  heap_init(&h, sizeof(int), NULL);

  BENCHMARK_FULL("Heap Push (Descending Values)", N_OPERATIONS, {
    int val = (int)N_OPERATIONS - (int)i;
    heap_push(&h, &val);
  });

  Iter it = heap_iter(&h);

  BENCHMARK_FULL("Heap Iteration", N_OPERATIONS, { it.next(&it); });

  BENCHMARK_FULL("Heap Pop", N_OPERATIONS, {
    int out;
    heap_pop(&h, &out);
  });

  heap_free(&h);
}

static void run_heapify() {
  Vector v;
  Heap h;
  vector_init(&v, sizeof(int));
  heap_init(&h, sizeof(int), NULL);
  for (int i = N_OPERATIONS; i >= 0; i--)
    vector_push(&v, &i);
  BENCHMARK_FULL("Heapify Vector (1M Descending Values)", 1,
                 { heap_heapify(&h, &v); });
  heap_free(&h);
}

int main() {
  printf("\nRunning: %s...\n", __FILE__);
  env_info();
  run_basic_heap_ops();
  run_heapify();
  return 0;
}
