#include "clib/test_framework.h"
#include "clib/vector.h"

int main() {
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
  test_summary();
  return tests_failed > 0 ? 1 : 0;
}
