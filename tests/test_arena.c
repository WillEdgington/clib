#include "clib/arena.h"
#include "clib/test_framework.h"
#include <stdint.h>

int main() {
  Arena a;
  // Initialize with a small slab size to force chaining easily
  arena_init(&a, 64);

  void *p1 = arena_alloc(&a, 8);
  void *p2 = arena_alloc(&a, 8);

  ASSERT_PTR_NOT_NULL(p1, "First allocation should succeed");
  ASSERT_PTR_NOT_NULL(p2, "Second allocation should succeed");

  // In a bump allocator, p2 should be p1 + 8
  size_t diff = (char *)p2 - (char *)p1;
  ASSERT_INT_EQ(diff, 8, "Allocations in the same slab should be contiguous");

  // Already used 16 bytes. A new request of 60 bytes should force chaining
  // (Total 76 > 64)
  void *p3 = arena_alloc(&a, 60);
  ASSERT_PTR_NOT_NULL(p3, "Allocation forcing a new slab should succeed");
  ASSERT(a.head != a.current, "Arena should have moved to a new region/slab");

  // Request something significantly larger than the default slab_size
  void *p4 = arena_alloc(&a, 1000);
  ASSERT_PTR_NOT_NULL(
      p4, "Large allocation should succeed by creating a custom-sized slab");

  arena_reset(&a);
  ASSERT_INT_EQ(a.head->offset, 0,
                "Head region offset should be 0 after reset");
  ASSERT(a.current == a.head,
         "Current region should be reset to head after reset");

  arena_free(&a);
  test_summary();
  return tests_failed > 0 ? 1 : 0;
}