#include "clib/arena.h"
#include "clib/hashmap.h"
#include "clib/metric_framework.h"
#include "clib/ohashmap.h"

#define ARENA_SLAB_SIZE 4096

static void env_info(size_t slab_size, double hashmap_lf, double ohashmap_lf) {
  printf("==========================================\n");
  printf("ENVIRONMENT SETUP:\n");
  printf("  Arena Slab Size: %zu bytes\n", slab_size);
  printf("  Max Load Factor: %.2f (chain), %.2f (open)\n", hashmap_lf,
         ohashmap_lf);
  printf("==========================================\n\n");
}

void run_workload(const char *label, void *map,
                  void (*insert_fn)(void *, int)) {
  BENCHMARK_FULL(label, 1000000, { insert_fn(map, i); });
}

// wrapper for Chaining
void wrap_chaining_put(void *m, int i) { hashmap_put((HashMap *)m, &i, &i); }

// wrapper for Open Addressing
void wrap_open_put(void *m, int i) { ohashmap_put((OHashMap *)m, &i, &i); }

int main() {
  printf("\nRunning: %s...\n", __FILE__);
  env_info((size_t)ARENA_SLAB_SIZE, (double)HASHMAP_LOAD_FACTOR,
           (double)OHASHMAP_LOAD_FACTOR);
  Arena a;
  arena_init(&a, ARENA_SLAB_SIZE);

  HashMap chaining;
  hashmap_init(&chaining, sizeof(int), sizeof(int), &a);
  run_workload("Hashmap (Chaining)", &chaining, wrap_chaining_put);
  hashmap_free(&chaining);

  arena_reset(&a);
  arena_free(&a);
  arena_init(&a, ARENA_SLAB_SIZE);

  OHashMap open;
  ohashmap_init(&open, sizeof(int), sizeof(int), &a);
  run_workload("Hashmap (Open Addressing)", &open, wrap_open_put);
  ohashmap_free(&open);

  arena_free(&a);
  return 0;
}