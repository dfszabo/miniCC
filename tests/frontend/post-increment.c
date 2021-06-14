// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 1
// TEST-CASE: test(1) -> 3
// TEST-CASE: test(2) -> 5


int test(int a) {
  int b;
  b = a++;
  return b + a;
}
