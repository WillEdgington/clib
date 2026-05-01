#include "clib/arena.h"
#include "clib/hashmap.h"
#include "clib/iter.h"
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

static void test_hashmap_ints(Arena *arena) {
  HashMap map;
  hashmap_init(&map, sizeof(int), sizeof(int), arena);

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
}

static void test_hashmap_strings(Arena *arena) {
  // String type (char *) keys (test custom function pointers)
  HashMap map;
  hashmap_init(&map, sizeof(char *), sizeof(int), arena);
  hashmap_set_functions(&map, str_hash, str_compare);

  const char *s_key = "hello";
  int s_val = 999;
  hashmap_put(&map, &s_key, &s_val);

  int *s_res = (int *)hashmap_get(&map, &s_key);
  ASSERT_PTR_NOT_NULL(s_res, "Should find string key 'hello'");
  ASSERT_INT_EQ(*s_res, 999, "Value should match 999");
  hashmap_free(&map);
}

static void test_hashmap_iter(Arena *arena) {
  HashMap map;
  hashmap_init(&map, sizeof(int), sizeof(int), arena);
  Iter pre_it = hashmap_iter(&map);

  for (int i = 0; i < 10; i++) {
    int v = i + 10;
    hashmap_put(&map, &i, &v);
  }

  Iter it = hashmap_iter(&map);
  int count = 0, correct = 0;
  while (it.next(&it) == 0) {
    int key = *(int *)it.current.key;
    int val = *(int *)it.current.value;
    if (val == *(int *)hashmap_get(&map, &key))
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
  hashmap_free(&map);
}

int main() {
  printf("\nTesting: %s...\n", __FILE__);
  Arena arena;
  arena_init(&arena, 1024); // 1KB slabs

  test_hashmap_ints(&arena);
  test_hashmap_strings(&arena);
  test_hashmap_iter(&arena);

  arena_free(&arena);
  test_summary();
  return tests_failed > 0 ? 1 : 0;
}