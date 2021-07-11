// RUN: AArch64
// FUNC-DECL: int test(int, int)
// TEST-CASE: test(0, 0) -> 23
// TEST-CASE: test(1, 0) -> 23
// TEST-CASE: test(1, 1) -> 66
// TEST-CASE: test(14, 1) -> 23

int test(int a, int b) {
  if (a > 0 && a < 13 && b > 0)
    return 66;
  return 23;
}
