// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 0
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(2) -> 3
// TEST-CASE: test(3) -> 6
// TEST-CASE: test(100) -> 5050

int test(int a) {
  int res = 0;
  for (int i = 0; i < a + 1; i++)
    res += i;
  return res;
}
