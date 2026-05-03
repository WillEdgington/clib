#include "clib/heap.h"
#include "clib/test_framework.h"

// MAX-HEAP: Larger value has higher priority (moves to top)
static int max_int_cmp(const void *a, const void *b) {
  return (*(int *)a > *(int *)b) ? 1 : 0;
}

// MIN-HEAP STRINGS: A-Z priority
static int min_str_cmp(const void *a, const void *b) {
  return strcmp(*(const char **)b, *(const char **)a) > 0 ? 1 : 0;
}

// MAX-HEAP STRINGS: Z-A priority
static int max_str_cmp(const void *a, const void *b) {
  return strcmp(*(const char **)a, *(const char **)b) > 0 ? 1 : 0;
}

static void test_heap_min_ints() {
  Heap h;
  heap_init(&h, sizeof(int), NULL); // compare of NULL sets to min int heap

  int values[] = {20, 10, 30, 5, 15};
  for (int i = 0; i < 5; i++)
    heap_push(&h, &values[i]);

  ASSERT_INT_EQ(*(int *)heap_peek(&h), 5,
                "Min-heap peek should be smallest int pushed (int 5)");

  int out, last = -1;
  int correct = 0, count = 0;
  for (int i = 0; i < 5; i++) {
    heap_pop(&h, &out);
    if (out >= last)
      correct++;
    count++;
    last = out;
  }
  ASSERT_INT_EQ(correct, count, "Min-heap sequence should be non-decreasing");
  heap_free(&h);
}

static void test_heap_max_ints() {
  Heap h;
  heap_init(&h, sizeof(int), max_int_cmp);

  int values[] = {20, 10, 30, 5, 15};
  for (int i = 0; i < 5; i++)
    heap_push(&h, &values[i]);

  ASSERT_INT_EQ(*(int *)heap_peek(&h), 30,
                "Max-heap peek should be largest int pushed (int 30)");

  int out, last = 100;
  int correct = 0, count = 0;
  for (int i = 0; i < 5; i++) {
    heap_pop(&h, &out);
    if (out <= last)
      correct++;
    count++;
    last = out;
  }
  ASSERT_INT_EQ(correct, count, "Max-heap sequence should be non-increasing");
  heap_free(&h);
}

static void test_heap_min_strings() {
  Heap h;
  heap_init(&h, sizeof(char *), min_str_cmp);

  const char *s1 = "cherry", *s2 = "apple", *s3 = "banana";
  heap_push(&h, &s1);
  heap_push(&h, &s2);
  heap_push(&h, &s3);

  char *out;
  heap_pop(&h, &out);
  ASSERT_STR_EQ(out, "apple",
                "Min-heap string should pop 'apple' first ('apple' < 'banana' "
                "< 'cherry')");
  heap_pop(&h, &out);
  ASSERT_STR_EQ(
      out, "banana",
      "Min-heap string should pop 'banana' second ('banana' < 'cherry')");

  heap_free(&h);
}

static void test_heap_max_strings() {
  Heap h;
  heap_init(&h, sizeof(char *), max_str_cmp);

  const char *s1 = "cherry", *s2 = "apple", *s3 = "banana";
  heap_push(&h, &s1);
  heap_push(&h, &s2);
  heap_push(&h, &s3);

  char *out;
  heap_pop(&h, &out);
  ASSERT_STR_EQ(out, "cherry",
                "Max-heap string should pop 'cherry' first ('cherry' > "
                "'banana' > 'apple')");

  heap_free(&h);
}

static void test_heap_min_iter() {
  Heap h;
  heap_init(&h, sizeof(int), NULL);
  Iter pre_it = heap_iter(&h);

  for (int i = 9; i >= 0; i--)
    heap_push(&h, &i);
  int lvl_ord[] = {0, 1, 4, 3, 2, 8, 5, 9, 6, 7};

  Iter it = heap_iter(&h);
  int correct = 0, i = 0;
  while (it.next(&it) == 0) {
    if (*(int *)it.current.value == lvl_ord[i])
      correct++;
    i++;
  }

  int pre_count = 0;
  while (pre_it.next(&pre_it) == 0)
    pre_count++;

  ASSERT_INT_EQ(correct, (int)h.data.count,
                "Iterator should iterate through heap at level order "
                "(traverses underlying Vector)");
  ASSERT_INT_EQ(pre_count, (int)h.data.count,
                "Iterator should iterate through items added after init");
  heap_free(&h);
}

int main() {
  printf("\nTesting: %s...\n", __FILE__);

  test_heap_min_ints();
  test_heap_max_ints();
  test_heap_min_strings();
  test_heap_max_strings();
  test_heap_min_iter();
  test_summary();

  return tests_failed > 0 ? 1 : 0;
}
