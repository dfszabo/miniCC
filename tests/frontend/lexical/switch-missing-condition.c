// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


int test(int a) {
  switch () {
  case 1:
    break;
  default:
    break;
  }
}
