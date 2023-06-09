#include <stdio.h>
#include <stdlib.h>

double int_x2(double a, double b, long n) {
  double s = 0.0;
  double x = a;
  double dx = (b - a) / (double)n;
  for (long i = 0; i < n; i++) {
    s += x * x;
    x += dx;
  }
  return s * dx;
}

int main(int argc, char ** argv) {
  double a = (argc > 1 ? atof(argv[1]) : 0.0);
  double b = (argc > 2 ? atof(argv[2]) : 1.0);
  long n   = (argc > 3 ? atof(argv[3]) : 1000L * 1000L * 1000L);
  double s = int_x2(a, b, n);
  printf("s = %f\n", s);
  return 0;
}
