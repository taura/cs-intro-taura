#include <stdio.h>
#include <stdlib.h>

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
  float x = ax_plus_b(a, b, 1.0, n);
  printf("x = %f\n", x);
  return 0;
}
