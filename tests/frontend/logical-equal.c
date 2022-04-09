// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(0, 1) -> 0
// TEST-CASE: test(1, 1) -> 1
// TEST-CASE: test(100, 101) -> 0
// TEST-CASE: test(100, 100) -> 1


int test(int a, int b) {
  return a == b;
}
