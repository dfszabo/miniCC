// COMPILE-FAIL

extern int b;

int test(int a) {
  return b + a;
}
