//% file: hello.c
//% cmd: gcc -o hello hello.c

#include <stdio.h>
int main() {
  printf("hello to stdout\n");
  fprintf(stderr, "good bye to stderr\n");
  return 0;
}

