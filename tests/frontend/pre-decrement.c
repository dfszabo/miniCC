// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> -2
// TEST-CASE: test(1) -> 0
// TEST-CASE: test(2) -> 2


int test(int a) {
  int b;
  b = --a;
  return b + a;
}
