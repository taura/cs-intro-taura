#include <stdio.h>
#include <stdlib.h>

typedef double real;
typedef real realv __attribute__((vector_size(32),aligned(sizeof(real))));

enum { nl = sizeof(realv) / sizeof(real) };

template<int nc>
real int_x2_simd_template(real a, real b, long n) {
  realv s[nc];
  realv x[nc];
  real dx = (b - a) / (real)n;
  real x_ = a;
  for (long i = 0; i < nc; i++) {
    for (long j = 0; j < nl; j++) {
      s[i][j] = 0.0;
    }
  }
  for (long i = 0; i < nc; i++) {
    for (long j = 0; j < nl; j++) {
      x[i][j] = a + (nl * i + j) * dx;
    }
  }
  asm volatile("# ============= int_x2_simd_template loop");
  for (long i = 0; i < n; i += nc * nl) {
    for (long j = 0; j < nc; j++) {
      s[j] += x[j] * x[j];
      x[j] += nc * nl * dx;
    }
  }
  asm volatile("# ------------- int_x2_simd_template loop");
  realv ss = s[0];
  for (long j = 1; j < nc; j++) {
    ss += s[j];
  }
  real sss = ss[0];
  for (long j = 1; j < nl; j++) {
    sss += ss[j];
  }
  return sss * dx;
}

int main(int argc, char ** argv) {
  real a = (argc > 1 ? atof(argv[1]) : 0.0);
  real b = (argc > 2 ? atof(argv[2]) : 1.0);
  long n   = (argc > 3 ? atof(argv[3]) : 1000L * 1000L * 1000L);
  real s = int_x2_simd_template<4>(a, b, n);
  printf("s = %f\n", s);
  return 0;
}
