// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(10, 3) -> 1
// TEST-CASE: test(100, 100) -> 1
// TEST-CASE: test(140, 155) -> 0

int test(int a, int b) {
  return a >= b;
}
