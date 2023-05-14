#include <stdio.h>
#include <stdlib.h>
#include "counter.h"

float ax_plus_b(float a, float b, float x, long n) {
  for (long j = 0; j < n; j++) {
    x = a * x + b;
  }
  return x;
}

int main(int argc, char ** argv) {
  long n = (argc > 1 ? atol(argv[1]) : 1000L * 1000L * 1000L);
  float a = (argc > 2 ? atof(argv[2]) : 0.999);
  float b = (argc > 3 ? atof(argv[3]) : 0.12345);
  profiler_t pr = mk_profiler();
  float x = ax_plus_b(a, b, 1.0, n);
  counters_t c = profiler_get(pr);
  printf("x = %f\n", x);
  for (int i = 0; i < c.n; i++) {
    printf("%20s: %ld\n", c.names[i], c.values[i]);
  }
  printf("%20s: %ld\n", "ref cycles", c.tsc);
  printf("%20s: %.3f\n", "time", c.ns * 1.0e-9);
  return 0;
}

