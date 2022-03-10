// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL

int func();

int test(int b) {
  func = b;
  return b;
}
