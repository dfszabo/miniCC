// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


int test(int b) {
  a = b;
  return a;
}
