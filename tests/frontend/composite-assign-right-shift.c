// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(10, 2) -> 2
// TEST-CASE: test(100, 3) -> 12
// TEST-CASE: test(255, 4) -> 15

int test(int a, int b) {
  a >>= b;
  return a;
}
