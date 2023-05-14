float ax_plus_b(float a, float b, float x, long n) {
  asm volatile("# ============= ax_plus_b");
  for (long j = 0; j < n; j++) {
    x = a * x + b;
  }
  asm volatile("# ------------- ax_plus_b");
  return x;
}

