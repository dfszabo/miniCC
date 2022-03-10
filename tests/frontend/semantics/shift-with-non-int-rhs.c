// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


int test(int b) {
  return b << 1.0;
}
