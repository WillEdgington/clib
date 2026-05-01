#include "clib/arena.h"
#include "clib/ohashmap.h"
#include "clib/test_framework.h"

#include <string.h>

// string comparison since strcmp has a different signature
static int str_compare(const void *k1, const void *k2, size_t key_size) {
  (void)key_size;
  return strcmp(*(const char **)k1, *(const char **)k2);
}

// simple string hash (DJB2)
static size_t str_hash(const void *key, size_t key_size) {
  (void)key_size;
  const char *str = *(const char **)key;
  size_t hash = 5381;
  int c;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;
  return hash;
}

// hash function that always returns 0 (collisions are guaranteed)
static size_t bad_hash(const void *key, size_t key_size) {
  (void)key_size;
  (void)key;
  return 0;
}

static void test_ohashmap_ints(Arena *arena) {
  OHashMap map;
  ohashmap_init(&map, sizeof(int), sizeof(int), arena);

  int key1 = 10, val1 = 100;
  int key2 = 20, val2 = 200;

  ohashmap_put(&map, &key1, &val1);
  ohashmap_put(&map, &key2, &val2);

  ASSERT_INT_EQ(ohashmap_count(&map), 2, "Map should contain 2 items");

  int *res1 = (int *)ohashmap_get(&map, &key1);
  ASSERT_PTR_NOT_NULL(res1, "Should find key 10");
  ASSERT_INT_EQ(*res1, 100, "Value for key 10 should be 100");

  // Update key1 entry
  int val1_new = 150;
  ohashmap_put(&map, &key1, &val1_new);
  int *res1_updated = (int *)ohashmap_get(&map, &key1);
  ASSERT_INT_EQ(*res1_updated, 150, "Value should be updated to 150");
  ASSERT_INT_EQ(ohashmap_count(&map), 2,
                "Count should still be 2 after update");

  // Delete an entry
  ohashmap_remove(&map, &key2);
  ASSERT_PTR_NULL(ohashmap_get(&map, &key2), "Key 20 should be gone");
  ASSERT_INT_EQ(ohashmap_count(&map), 1, "Count should be 1 after removal");

  // Force a resize by inserting more items than the initial capacity
  // (default 16) * 0.7
  for (int i = 0; i < 20; i++) {
    int k = i + 1000;
    int v = i;
    ohashmap_put(&map, &k, &v);
  }
  ASSERT(map.capacity > 16, "Map should have resized/grown");
  int find_key = 1010;
  int *find_val = (int *)ohashmap_get(&map, &find_key);
  ASSERT_PTR_NOT_NULL(find_val, "Should still find items after resize");
  ASSERT_INT_EQ(*find_val, 10, "Value should be correct after resize");

  ohashmap_free(&map);
}

static void test_ohashmap_strings(Arena *arena) {
  // String type (char *) keys (test custom function pointers)
  OHashMap map;
  ohashmap_init(&map, sizeof(char *), sizeof(int), arena);
  ohashmap_set_functions(&map, str_hash, str_compare);

  const char *s_key = "hello";
  int s_val = 999;
  ohashmap_put(&map, &s_key, &s_val);

  int *s_res = (int *)ohashmap_get(&map, &s_key);
  ASSERT_PTR_NOT_NULL(s_res, "Should find string key 'hello'");
  ASSERT_INT_EQ(*s_res, 999, "Value should match 999");
  ohashmap_free(&map);
}

static void test_ohashmap_iters(Arena *arena) {
  OHashMap map;
  ohashmap_init(&map, sizeof(int), sizeof(int), arena);
  Iter pre_it = ohashmap_iter(&map);

  for (int i = 0; i < 10; i++) {
    int v = i + 10;
    ohashmap_put(&map, &i, &v);
  }

  Iter it = ohashmap_iter(&map);
  int count = 0, correct = 0;
  while (it.next(&it) == 0) {
    int key = *(int *)it.current.key;
    int val = *(int *)it.current.value;
    if (val == *(int *)ohashmap_get(&map, &key))
      correct++;
    count++;
  }

  int pre_count = 0;
  while (pre_it.next(&pre_it) == 0)
    pre_count++;

  ASSERT_INT_EQ(count, (int)map.count,
                "Iterator should iterate through the whole vector");
  ASSERT_INT_EQ(correct, (int)map.count,
                "All key/value pairs traversed by the iterator should match "
                "the expected");
  ASSERT_INT_EQ(
      pre_count, (int)map.count,
      "Iterator should iterate through key/value pairs added after init");
  ohashmap_free(&map);
}

static void test_ohashmap_collisions(Arena *arena) {
  OHashMap map;
  ohashmap_init(&map, sizeof(int), sizeof(int), arena);

  // use bad_hash to guarantee collisions
  ohashmap_set_functions(&map, bad_hash, NULL);

  int k1 = 5, v1 = 500;
  int k2 = 21, v2 = 2100;

  // force two keys into the same ideal bucket allocation
  ohashmap_put(&map, &k1, &v1);
  ohashmap_put(&map, &k2, &v2);
  ASSERT_INT_EQ(map.count, 2,
                "Count should be 2 after put operation of kv pair: k2 (int "
                "21), v2 (int 2100)");
  ASSERT_INT_EQ(*(int *)ohashmap_get(&map, &k2), 2100,
                "Should find collided key k2 (int 21)");

  // Tombstone test: remove k1. This creates a Tombstone. k2 should still be
  // found.
  ohashmap_remove(&map, &k1);
  ASSERT_PTR_NULL(ohashmap_get(&map, &k1), "Key k1 (int 5) should be removed");
  ASSERT_PTR_NOT_NULL(ohashmap_get(&map, &k2),
                      "Key k2 (int 21) should still be found past a tombstone");

  // Tombstone Reclamation: put new key in the tombstone bucket left by k1
  int k3 = 37, v3 = 3700;
  ohashmap_put(&map, &k3, &v3);
  ASSERT_INT_EQ(
      map.count, 2,
      "Count should be 2 after removing k1 (int 5) and adding k3 (int 37)");
  ohashmap_free(&map);
}

int main() {
  printf("\nTesting: %s...\n", __FILE__);
  Arena arena;
  arena_init(&arena, 1024); // 1 KB slab

  test_ohashmap_ints(&arena);
  test_ohashmap_strings(&arena);
  test_ohashmap_iters(&arena);
  test_ohashmap_collisions(&arena);

  arena_free(&arena);
  test_summary();
  return tests_failed > 0 ? 1 : 0;
}