#include "clib/arena.h"
#include "clib/hashmap.h"
#include "clib/metric_framework.h"
#include "clib/ohashmap.h"

#define ARENA_SLAB_SIZE 4096
#define N_OPERATIONS 1000000

static void env_info(size_t slab_size, double hashmap_lf, double ohashmap_lf) {
  printf("==========================================\n");
  printf("ENVIRONMENT SETUP:\n");
  printf("  Arena Slab Size: %zu bytes\n", slab_size);
  printf("  Max Load Factor: %.2f (chain), %.2f (open)\n", hashmap_lf,
         ohashmap_lf);
  printf("==========================================\n\n");
}

int main() {
  printf("\nRunning: %s...\n", __FILE__);
  env_info((size_t)ARENA_SLAB_SIZE, (double)HASHMAP_LOAD_FACTOR,
           (double)OHASHMAP_LOAD_FACTOR);
  Arena a;
  arena_init(&a, ARENA_SLAB_SIZE);

  HashMap chaining;
  hashmap_init(&chaining, sizeof(int), sizeof(int), &a);
  BENCHMARK_FULL("Hashmap Put (Chaining)", N_OPERATIONS,
                 { hashmap_put(&chaining, &i, &i); });
  BENCHMARK_FULL("Hashmap Get (Chaining)", N_OPERATIONS,
                 { hashmap_get(&chaining, &i); });
  hashmap_free(&chaining);

  arena_reset(&a);
  arena_free(&a);
  arena_init(&a, ARENA_SLAB_SIZE);

  OHashMap open;
  ohashmap_init(&open, sizeof(int), sizeof(int), &a);
  BENCHMARK_FULL("Hashmap Put (Open Addressing)", N_OPERATIONS,
                 { ohashmap_put(&open, &i, &i); });
  BENCHMARK_FULL("Hashmap Get (Open Addressing)", N_OPERATIONS,
                 { ohashmap_get(&open, &i); });
  ohashmap_free(&open);

  arena_free(&a);
  return 0;
}