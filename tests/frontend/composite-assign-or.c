// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(10, 3) -> 11
// TEST-CASE: test(100, 12) -> 108
// TEST-CASE: test(140, 15) -> 143

int test(int a, int b) {
  a |= b;
  return a;
}
