// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 2
// TEST-CASE: test(1) -> 4
// TEST-CASE: test(2) -> 6


int test(int a) {
  int b;
  b = ++a;
  return b + a;
}
