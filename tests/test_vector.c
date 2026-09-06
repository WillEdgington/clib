#include "clib/iter.h"
#include "clib/test_framework.h"
#include "clib/vector.h"

static void test_vector_ints(void) {
  Vector v;
  int val1 = 100;
  int val2 = 200;
  int popped;

  vector_init(&v, sizeof(int));

  vector_push(&v, &val1);
  vector_push(&v, &val2);
  ASSERT_INT_EQ(v.count, 2,
                "Vector count should be 2 after two pushes (int 100, 200)");

  int result = vector_pop(&v, &popped);
  ASSERT_INT_EQ(result, 0, "Pop should return 0 (success)");
  ASSERT_INT_EQ(popped, 200,
                "Popped value should be the last pushed value (int 200)");
  ASSERT_INT_EQ(v.count, 1, "Count should decrement by one after pop (to 1)");

  result = vector_pop(&v, &popped);
  ASSERT_INT_EQ(result, 0, "Pop should succeed when emptying the vector");
  ASSERT_INT_EQ(popped, 100,
                "Popped value should be first pushed value (int 100)");
  ASSERT_INT_EQ(v.count, 0, "Count should decrement by one after pop (to 0)");

  result = vector_pop(&v, &popped);
  ASSERT_INT_EQ(result, -1,
                "Pop should return -1 when vector is empty (error)");

  vector_free(&v);

  ASSERT_INT_EQ(v.count, 0, "Vector items count should be reset after vector_free");
  ASSERT_INT_EQ(v.item_size, 0, "Vector item size metadata should be reset after vector_free");
  ASSERT_INT_EQ(v.capacity, 0, "Vector capacity should be reset after vector_free");
}

static void test_vector_strings(void) {
  Vector v;
  vector_init(&v, sizeof(char *));

  const char *s1 = "Hello";
  const char *s2 = "World";

  vector_push(&v, &s1);
  vector_push(&v, &s2);

  char *popped;
  vector_pop(&v, &popped);
  ASSERT_STR_EQ(popped, s2, "Vector should handle string pointers (pop)");
  vector_free(&v);
}

static void test_vector_iter(void) {
  Vector v;
  vector_init(&v, sizeof(int));
  Iter pre_it = vector_iter(&v);

  for (int i = 0; i < 10; i++) {
    vector_push(&v, &i);
  }

  Iter it = vector_iter(&v);
  int count = 0, correct = 0;
  while (it.next(&it) == 0) {
    int val = *(int *)it.current.value;
    if (val == count)
      correct++;
    count++;
  }

  int pre_count = 0;
  while (pre_it.next(&pre_it) == 0)
    pre_count++;

  ASSERT_INT_EQ(count, (int)v.count,
                "Iterator should iterate through the whole vector");
  ASSERT_INT_EQ(
      correct, (int)v.count,
      "All items traversed by the iterator should match the expected");
  ASSERT_INT_EQ(pre_count, (int)v.count,
                "Iterator should iterate through items pushed after init");
  vector_free(&v);
}

static void test_vector_remove(void) {
  Vector v;
  vector_init(&v, sizeof(int));

  int zero = 0;
  vector_push(&v, &zero);
  int one = 1;
  vector_push(&v, &one);
  int two = 2;
  vector_push(&v, &two);

  int ptr_val = 54;
  // Standard case:
  // vector_remove returns 0
  // item at index is copied to ptr
  // vector.count is decreased by one
  // items are in their expected place (everything that was after index was moved down)
  ASSERT_INT_EQ(vector_remove(&v, 1, &ptr_val), 0, "vector_remove() should return 0 (success) if able to remove at index");
  ASSERT_INT_EQ(ptr_val, 1, "The removed item should be copied to the ptr");
  ASSERT_INT_EQ(v.count, 2, "Item count should have decreased by one after successful vector_remove() call");
  ASSERT_INT_EQ(*(int *)vector_get(&v, 0), 0, "Item before removed index (index - 1) should be as expected after vector_remove() (unchanged)");
  ASSERT_INT_EQ(*(int *)vector_get(&v, 1), 2, "Item at index should be the item that was after index (index + 1) before vector_remove() call");
  
  // OOB case:
  // out of bound index returns -1
  // item count should remain the same
  ASSERT_INT_EQ(vector_remove(&v, 2, &ptr_val), -1, "vector_remove() call using index equal to the item count (OOB) should return -1 (error)");
  ASSERT_INT_EQ(vector_remove(&v, 3595, &ptr_val), -1, "vector_remove() call using index that is obviously OOB should return -1 (error)");
  ASSERT_INT_EQ(v.count, 2, "Item count should remain the same for vector used in failed vector_remove() call");

  // NULL pointer for v:
  // should return -1
  ASSERT_INT_EQ(vector_remove(NULL, 0, &ptr_val), -1, "vector_remove() call with NULL pointer for the vector pointer should return -1 (error)");

  // NULL ptr case:
  // returns 0?
  // item should still be removed? (count decreased, etc)
  ASSERT_INT_EQ(vector_remove(&v, 1, NULL), 0, "vector_remove() call with NULL given for the copy pointer (ptr) should succeed (return 0)");
  ASSERT_INT_EQ(v.count, 1, "Item count for vector should decrease after vector_remove() call with ptr == NULL");
  ASSERT_INT_EQ(*(int *)vector_get(&v, 0), 0, "remaining items should be at the expected indexes after vector_remove() call with ptr == NULL");
  vector_free(&v);
}

int main(void) {
  printf("\nTesting: %s...\n", __FILE__);
  test_vector_ints();
  test_vector_strings();
  test_vector_iter();
  test_vector_remove();
  test_summary();
  return tests_failed > 0 ? 1 : 0;
}
