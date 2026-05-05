#ifndef METRIC_FRAMEWORK_H
#define METRIC_FRAMEWORK_H

// informs gcc compiler to expose POSIX.1b (Real-time) features
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// returns current Resident Set Size (RSS) in KB
static inline long get_memory_usage_kb() {
  long rss = 0L;
  FILE *fp = fopen("/proc/self/status", "r");
  if (fp) {
    char line[128];
    while (fgets(line, 128, fp)) {
      if (strncmp(line, "VmRSS:", 6) == 0) {
        rss = strtol(line + 6, NULL, 10);
        break;
      }
    }
    fclose(fp);
  }
  return rss;
}

#define BENCHMARK_FULL(name, iterations, block)                                \
  do {                                                                         \
    long mem_before = get_memory_usage_kb();                                   \
    struct timespec start, end;                                                \
    clock_gettime(CLOCK_MONOTONIC, &start);                                    \
    for (size_t i = 0; i < (size_t)iterations; i++) {                          \
      block;                                                                   \
    }                                                                          \
    clock_gettime(CLOCK_MONOTONIC, &end);                                      \
    long mem_after = get_memory_usage_kb();                                    \
    double elapsed =                                                           \
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;     \
    printf("--- %s ---\n", name);                                              \
    printf("  Time:   %.4fs (%zu iterations)\n", elapsed, (size_t)iterations); \
    printf("  Speed:  %.2fns/op\n", (elapsed * 1e9) / iterations);             \
    printf("  Memory: %ld KB (Delta: %+ld KB)\n\n", mem_after,                 \
           mem_after - mem_before);                                            \
  } while (0)

#endif
