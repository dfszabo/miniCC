// COMPILE-FAIL

int test(int a) {
  return a * *a;
}
