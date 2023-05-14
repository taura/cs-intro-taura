#include <stdio.h>
#include <stdlib.h>

/* double/float をこれだけで変更できるように */
typedef double real;

/* realを32バイト分 (float x 8 または double x 4)並べたデータ型 */
typedef real realv __attribute__((vector_size(64),aligned(sizeof(real))));

/* number of lanes (1 SIMD変数あたりの要素数) */
enum { nl = sizeof(realv) / sizeof(real) };

real int_x2_simd_v(real a, real b, long n) {
  real dx = (b - a) / (real)n;
  realv s;
  realv x;
  /* sの全要素を0に */
  for (long i = 0; i < nl; i++) {
    s[i] = 0.0;
  }
  /* x = { a, a+dx, a+2*dx, ... } */
  for (long i = 0; i < nl; i++) {
    x[i] = a + i * dx;
  }
  /* 本題 */
  for (long i = 0; i < n; i += nl) {
    s += x * x;
    x += nl * dx;
  }
  /* sの各要素に入った部分和を足し合わせる */
  real ss = 0.0;
  for (long i = 0; i < nl; i++) {
    ss += s[i];
  }
  return ss  * dx;
}

int main(int argc, char ** argv) {
  real a = (argc > 1 ? atof(argv[1]) : 0.0);
  real b = (argc > 2 ? atof(argv[2]) : 1.0);
  long n   = (argc > 3 ? atof(argv[3]) : 1000L * 1000L * 1000L);
  real s = int_x2_simd_v(a, b, n);
  printf("s = %f\n", s);
  return 0;
}
