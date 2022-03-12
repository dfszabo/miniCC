// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


int test(int a) {
  switch (a) {
  case :
    break;
  default:
    break;
  }
}
