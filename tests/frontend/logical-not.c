// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 1
// TEST-CASE: test(1) -> 0
// TEST-CASE: test(123) -> 0

int test(int a) {
  return !a;
}
