// COMPILE-FAIL

int test(int a) {
  a = a % 1.0;
  return a;
}
