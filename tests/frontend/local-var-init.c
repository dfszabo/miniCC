// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 2
// TEST-CASE: test(1) -> 3
// TEST-CASE: test(2) -> 4

int test(int p) {
  int a = p + 2;
  return a;
}
