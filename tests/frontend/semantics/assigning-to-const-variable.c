// COMPILE-FAIL

const int a;

int test(int b) {
  a = b;
  return a;
}
