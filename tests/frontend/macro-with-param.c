// RUN: AArch64
// FUNC-DECL: int test(int, int)
// TEST-CASE: test(1, 2) -> 2
// TEST-CASE: test(1, -2) -> 1
// TEST-CASE: test(-3, -2) -> -2

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

int test(int a, int b) {
  return MAX(a, b);
}
