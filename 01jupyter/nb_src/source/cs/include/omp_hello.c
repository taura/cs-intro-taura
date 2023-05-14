//% file: omp_hello.c
//% cmd: gcc -O3 -fopenmp omp_hello.c -o omp_hello

#include <stdio.h>

int main() {
  printf("hello\n");
#pragma omp parallel
  printf("world\n");
  printf("good bye\n");
  return 0;
}

