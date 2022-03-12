// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


int test(int a) {
  if()
    return 1;
  return a;
}
