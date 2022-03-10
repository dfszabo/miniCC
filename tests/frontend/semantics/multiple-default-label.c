// RUN: AArch64

// EXTRA-FLAGS: -Wall
// FUNC-DECL: int test(int)
// COMPILE-FAIL

int test(int a) {
  switch (a) {
  case 1:
    return 1;
  default:
    return 2;
  default:
    return 3;
  }
}
