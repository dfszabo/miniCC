// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(1) -> -2
// TEST-CASE: test(100) -> -101
// TEST-CASE: test(-1) -> 0

unsigned test(unsigned a) {
  return ~a;
}
