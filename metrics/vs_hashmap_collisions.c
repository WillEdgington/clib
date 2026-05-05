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

// hash function that always returns 0 (collisions are guaranteed)
static size_t bad_hash(const void *key, size_t len) {
  (void)key;
  (void)len;
  return 0;
}

static void run_workload(const char *label, void *map,
                         void (*insert_fn)(void *, int)) {
  BENCHMARK_FULL(label, 10000, { insert_fn(map, i); });
}

// wrapper for Chaining
static void wrap_chaining_put(void *m, int i) {
  hashmap_put((HashMap *)m, &i, &i);
}

// wrapper for Open Addressing
static void wrap_open_put(void *m, int i) {
  ohashmap_put((OHashMap *)m, &i, &i);
}

int main() {
  printf("\nRunning: %s...\n", __FILE__);
  env_info((size_t)ARENA_SLAB_SIZE, (double)HASHMAP_LOAD_FACTOR,
           (double)OHASHMAP_LOAD_FACTOR);

  Arena a;
  arena_init(&a, ARENA_SLAB_SIZE);

  HashMap chaining;
  hashmap_init(&chaining, sizeof(int), sizeof(int), &a);
  hashmap_set_functions(&chaining, bad_hash, NULL);

  run_workload("Chaining (100% Collisions)", &chaining, wrap_chaining_put);
  hashmap_free(&chaining);

  arena_reset(&a);
  arena_free(&a);
  arena_init(&a, ARENA_SLAB_SIZE);

  OHashMap open;
  ohashmap_init(&open, sizeof(int), sizeof(int), &a);
  ohashmap_set_functions(&open, bad_hash, NULL);

  run_workload("Open Addressing (100% Collisions)", &open, wrap_open_put);
  ohashmap_free(&open);

  arena_free(&a);
  return 0;
}