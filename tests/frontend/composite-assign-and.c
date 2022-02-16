// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(10, 3) -> 2
// TEST-CASE: test(100, 12) -> 4
// TEST-CASE: test(140, 15) -> 12

int test(int a, int b) {
  a &= b;
  return a;
}
