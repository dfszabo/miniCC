// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(10, 3) -> 1
// TEST-CASE: test(100, 12) -> 4
// TEST-CASE: test(123, 100) -> 23

int test(int a, int b) {
  a %= b;
  return a;
}
