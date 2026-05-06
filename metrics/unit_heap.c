#include "clib/heap.h"
#include "clib/metric_framework.h"

#define N_OPERATIONS 1000000

static void env_info() {
  printf("==========================================\n");
  printf("ENVIRONMENT SETUP:\n");
  printf("  Min-Heap: int\n");
  printf("==========================================\n\n");
}

int main() {
  printf("\nRunning: %s...\n", __FILE__);
  env_info();
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
  return 0;
}