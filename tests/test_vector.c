#include "clib/iter.h"
#include "clib/test_framework.h"
#include "clib/vector.h"

static void test_vector_ints() {
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
}

static void test_vector_strings() {
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

static void test_vector_iter() {
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

int main() {
  printf("\nTesting: tests/test_vector.c...\n");
  test_vector_ints();
  test_vector_strings();
  test_vector_iter();
  test_summary();
  return tests_failed > 0 ? 1 : 0;
}
