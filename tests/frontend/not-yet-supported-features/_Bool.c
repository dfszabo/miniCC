// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


_Bool test(int a) {
  return a == 1;
}
