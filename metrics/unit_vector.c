#include "clib/metric_framework.h"
#include "clib/vector.h"

#define N_OPERATIONS 1000000

static void env_info() {
  printf("==========================================\n");
  printf("ENVIRONMENT SETUP:\n");
  printf("  Vector: int\n");
  printf("==========================================\n\n");
}

int main() {
  printf("\nRunning: %s...\n", __FILE__);
  env_info();
  Vector v;
  vector_init(&v, sizeof(int));

  BENCHMARK_FULL("Vector Push", N_OPERATIONS, {
    int val = (int)i;
    vector_push(&v, &val);
  });

  BENCHMARK_FULL("Vector Get", N_OPERATIONS, { vector_get(&v, (size_t)i); });

  Iter it = vector_iter(&v);
  BENCHMARK_FULL("Vector Iteration", N_OPERATIONS, { it.next(&it); });

  BENCHMARK_FULL("Vector Pop", N_OPERATIONS, {
    int out;
    vector_pop(&v, &out);
  });
  vector_free(&v);
  return 0;
}