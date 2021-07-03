// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 0
// TEST-CASE: test(1) -> -1
// TEST-CASE: test(-1) -> 1

int test(int a) {
  return -a;
}
