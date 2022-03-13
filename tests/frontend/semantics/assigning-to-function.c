// COMPILE-FAIL

int func();

int test(int b) {
  func = b;
  return b;
}
