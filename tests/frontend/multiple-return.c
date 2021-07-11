// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 0
// TEST-CASE: test(10) -> -10
// TEST-CASE: test(11) -> 11

int test(int a) {
  if (a > 10)
    return a;
  return -a;
}
