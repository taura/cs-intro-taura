#include <stdio.h>
#include <stdlib.h>

typedef double real;

real ax_plus_b_many(real a, real * X,
                    long m, long s, long n) {
  long mask = m - 1;
  real y = 0.0;
  asm volatile("# ============= inner_prod loop");
  for (long i = 0; i < n; i++) {
    for (long j = 0; j < m * s; j += s) {
      y += a * X[j & mask];
    }
  }
  asm volatile("# ------------- inner_prod loop");
  return y;
}

int main(int argc, char ** argv) {
  long log_m  = (argc > 1 ? atol(argv[1]) : 24); /* 16M = 64MB */
  long s      = (argc > 2 ? atol(argv[2]) : 1);
  long log_n  = (argc > 3 ? atol(argv[3]) : 6);
  real a     = (argc > 4 ? atof(argv[4]) : 0.999);
  /* m要素のrealの配列を作り適当に初期化 */
  long m = 1 << log_m;
  long n = 1 << log_n;
  printf("scan %ld elements %ld times (%ld accesses) with stride %ld\n",
         m, n, m * n, s);
  real * X = (real *)malloc(sizeof(real) * m);
  //unsigned short rg[3] = { j, j + 1, j + 2 };
  for (long i = 0; i < m; i++) {
    X[i] = 1; //2 * erand48(rg) - 1.0;
  }
  real y = ax_plus_b_many(a, X, m, s, n);
  /* 適当な要素を表示(計算を省略させないために) */
  printf("y = %f\n", y);
  return 0;
}
