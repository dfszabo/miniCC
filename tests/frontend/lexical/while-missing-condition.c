// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


int test(int a) {
  while() {
    return a;
  }
}
