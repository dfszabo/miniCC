// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(10, 3) -> 80
// TEST-CASE: test(100, 12) -> 409600
// TEST-CASE: test(140, 15) -> 4587520

int test(int a, int b) {
  a <<= b;
  return a;
}
