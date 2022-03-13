// COMPILE-FAIL

union {
  int a;
  char c;
} o;

int test(int a) {
  return o.a + a;
}
