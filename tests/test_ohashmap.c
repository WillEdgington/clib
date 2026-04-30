#include "clib/arena.h"
#include "clib/ohashmap.h"
#include "clib/test_framework.h"

#include <string.h>

int main() {
  Arena arena;
  arena_init(&arena, 2048);

  OHashMap map;
  ohashmap_init(&map, sizeof(int), sizeof(int), &arena);

  // Basic put/get test
  int k1 = 5, v1 = 500;
  ohashmap_put(&map, &k1, &v1);
  int *res = ohashmap_get(&map, &k1);
  ASSERT_PTR_NOT_NULL(res, "Should find key k1 (int 5)");
  ASSERT_INT_EQ(*res, 500, "Value should be v1 (int 500)");

  // Collision/Probing test: force keys that likely land in the same bucket
  // area. k1: 00000101, k2: 00010101
  int k2 = 21, v2 = 2100;
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

  // Tombstone Reclamation: put new key in similar bucket area to k1.
  // Ideally it will reclaim the Tombstone left by k1.
  // k1: 00000101, k3: 00100101
  int k3 = 37, v3 = 3700;
  ohashmap_put(&map, &k3, &v3);
  ASSERT_INT_EQ(
      map.count, 2,
      "Count should be 2 after removing k1 (int 5) and adding k3 (int 37)");

  ohashmap_free(&map);
  arena_free(&arena);

  test_summary();
  return tests_failed > 0 ? 1 : 0;
}