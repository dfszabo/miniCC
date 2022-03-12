// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


int test(int a) {
  _Atomic int b;
  return a;
}
