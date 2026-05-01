#include "clib/iter.h"
#include "clib/test_framework.h"

int string_iter_next(Iter *self) {
  char *str = (char *)self->collection;
  if (str[self->index] == '\0')
    return 1;
  self->current.value = &str[self->index];
  self->index++;
  return 0;
}

static void test_generic_contract() {
  char *my_str = "CLIB";
  Iter it = {.collection = my_str, .index = 0, .next = string_iter_next};
  char expected[] = {'C', 'L', 'I', 'B'};
  int correct = 0, count = 0;

  while (it.next(&it) == 0) {
    char val = *(char *)it.current.value;
    if (val == expected[count])
      correct++;
    count++;
  }

  ASSERT_INT_EQ(count, 4, "Should have iterated 4 characters (C, L, I, B)");
  ASSERT_INT_EQ(correct, 4, "Iterated characters should match the expected");
}

int main() {
  printf("\nTesting: %s...\n", __FILE__);
  test_generic_contract();
  test_summary();
  return tests_failed > 0 ? 1 : 0;
}