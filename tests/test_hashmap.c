#include "clib/arena.h"
#include "clib/hashmap.h"
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

int main() {
  Arena arena;
  arena_init(&arena, 1024); // 1KB slabs

  HashMap map;
  hashmap_init(&map, sizeof(int), sizeof(int), &arena);

  int key1 = 10, val1 = 100;
  int key2 = 20, val2 = 200;

  hashmap_put(&map, &key1, &val1);
  hashmap_put(&map, &key2, &val2);

  ASSERT_INT_EQ(hashmap_count(&map), 2, "Map should contain 2 items");

  int *res1 = (int *)hashmap_get(&map, &key1);
  ASSERT_PTR_NOT_NULL(res1, "Should find key 10");
  ASSERT_INT_EQ(*res1, 100, "Value for key 10 should be 100");

  // Update key1 entry
  int val1_new = 150;
  hashmap_put(&map, &key1, &val1_new);
  int *res1_updated = (int *)hashmap_get(&map, &key1);
  ASSERT_INT_EQ(*res1_updated, 150, "Value should be updated to 150");
  ASSERT_INT_EQ(hashmap_count(&map), 2, "Count should still be 2 after update");

  // Delete an entry
  hashmap_remove(&map, &key2);
  ASSERT_PTR_NULL(hashmap_get(&map, &key2), "Key 20 should be gone");
  ASSERT_INT_EQ(hashmap_count(&map), 1, "Count should be 1 after removal");

  // String type (char *) keys (test custom function pointers)
  HashMap smap;
  hashmap_init(&smap, sizeof(char *), sizeof(int), &arena);
  hashmap_set_functions(&smap, str_hash, str_compare);

  const char *s_key = "hello";
  int s_val = 999;
  hashmap_put(&smap, &s_key, &s_val);

  int *s_res = (int *)hashmap_get(&smap, &s_key);
  ASSERT_PTR_NOT_NULL(s_res, "Should find string key 'hello'");
  ASSERT_INT_EQ(*s_res, 999, "Value should match 999");

  // Force a resize by inserting more items than the initial capacity
  // (default 16) * 0.7
  for (int i = 0; i < 20; i++) {
    int k = i + 1000;
    int v = i;
    hashmap_put(&map, &k, &v);
  }
  ASSERT(map.capacity > 16, "Map should have resized/grown");
  int find_key = 1010;
  int *find_val = (int *)hashmap_get(&map, &find_key);
  ASSERT_PTR_NOT_NULL(find_val, "Should still find items after resize");
  ASSERT_INT_EQ(*find_val, 10, "Value should be correct after resize");

  hashmap_free(&map);
  hashmap_free(&smap);
  arena_free(&arena);

  test_summary();
  return tests_failed > 0 ? 1 : 0;
}