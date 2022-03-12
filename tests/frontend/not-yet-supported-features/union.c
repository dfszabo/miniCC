// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL

union {
  int a;
  char c;
} o;

int test(int a) {
  return o.a + a;
}
