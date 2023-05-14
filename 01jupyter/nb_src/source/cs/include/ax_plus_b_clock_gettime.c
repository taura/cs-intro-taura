#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long cur_time_ns() {
  struct timespec ts[1];
  clock_gettime(CLOCK_REALTIME, ts);
  return ts->tv_sec * 1000000000L + ts->tv_nsec;
}

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
  long t0 = cur_time_ns();
  float x = ax_plus_b(a, b, 1.0, n);
  long t1 = cur_time_ns();
  long dt = t1 - t0;
  printf("x = %f\n", x);
  printf("elapsed %ld nano sec\n", dt);
  printf("%f nano sec/fmadd\n", dt/(double)n);
  return 0;
}
