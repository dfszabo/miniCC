// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(10, 3) -> 9
// TEST-CASE: test(100, 12) -> 104
// TEST-CASE: test(140, 15) -> 131

int test(int a, int b) {
  a ^= b;
  return a;
}
