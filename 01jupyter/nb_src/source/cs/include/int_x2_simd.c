#include <stdio.h>
#include <stdlib.h>

/* doubleを8つ並べたデータ型(doublev) */
typedef double doublev __attribute__((vector_size(64),aligned(sizeof(double))));
enum { n_lanes = sizeof(doublev) / sizeof(double) };

double int_x2_simd(double a, double b, long n) {
  doublev s = {0,0,0,0,0,0,0,0};
  // n をレーン数の倍数に
  n += n_lanes - 1;
  n -= n % n_lanes;
  double dx = (b - a) / (double)n;
  doublev x = {a,a+dx,a+2*dx,a+3*dx,a+4*dx,a+5*dx,a+6*dx,a+7*dx};
  for (long i = 0; i < n; i += n_lanes) {
    s += x * x;
    x += n_lanes * dx;
  }
  return (s[0] + s[1] + s[2] + s[3] + s[4] + s[5] + s[6] + s[7]) * dx;
}

int main(int argc, char ** argv) {
  double a = (argc > 1 ? atof(argv[1]) : 0.0);
  double b = (argc > 2 ? atof(argv[2]) : 1.0);
  long n   = (argc > 3 ? atof(argv[3]) : 1000L * 1000L * 1000L);
  double s = int_x2_simd(a, b, n);
  printf("s = %f\n", s);
  return 0;
}
