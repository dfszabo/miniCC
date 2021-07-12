// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 0
// TEST-CASE: test(10) -> 10
// TEST-CASE: test(15) -> 10
// TEST-CASE: test(20) -> 15

// FUNC-DECL: int test_2(int)
// TEST-CASE: test_2(0) -> 22
// TEST-CASE: test_2(1) -> 11

int test(int a) {
  int res;
  res = (a > 10) ? a - 5 : a;
  return res;
}

int test_2(int a) {
  int res;
  res = (a) ? 11 : 22;
  return res;
}
