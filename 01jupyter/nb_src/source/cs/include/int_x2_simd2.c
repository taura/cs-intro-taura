#include <stdio.h>
#include <stdlib.h>
typedef double doublev __attribute__((vector_size(64),aligned(sizeof(double))));

double int_x2_simd_ilp(double a, double b, long n) {
  doublev s0 = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  doublev s1 = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  double dx = (b - a) / (double)n;
  doublev x0 = { a,      a+  dx, a+ 2*dx, a+ 3*dx, a+ 4*dx, a+ 5*dx, a+ 6*dx, a+ 7*dx };
  doublev x1 = { a+8*dx, a+9*dx, a+10*dx, a+11*dx, a+12*dx, a+13*dx, a+14*dx, a+15*dx };
  for (long i = 0; i < n; i += 16) {
    s0 += x0 * x0;
    s1 += x1 * x1;
    x0 +=  16 * dx;
    x1 +=  16 * dx;
  }
  doublev s = s0 + s1;
  return (s[0] + s[1] + s[2] + s[3] + s[4] + s[5] + s[6] + s[7]) * dx;
}

int main(int argc, char ** argv) {
  double a = (argc > 1 ? atof(argv[1]) : 0.0);
  double b = (argc > 2 ? atof(argv[2]) : 1.0);
  long n   = (argc > 3 ? atof(argv[3]) : 1000L * 1000L * 1000L);
  double s = int_x2_simd_ilp(a, b, n);
  printf("s = %f\n", s);
  return 0;
}
