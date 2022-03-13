// COMPILE-FAIL

int test(int a) {
  _Atomic int b;
  return a;
}
