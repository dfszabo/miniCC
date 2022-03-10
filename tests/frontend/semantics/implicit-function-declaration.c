// RUN: AArch64

// EXTRA-FLAGS: -Wall
// FUNC-DECL: int test()
// COMPILE-FAIL

int test() {
  return func();
}
