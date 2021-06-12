// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 0
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(2) -> 2
// TEST-CASE: test(3) -> 3
// TEST-CASE: test(4) -> 10

enum { A, B, C, D };

int test(int a) {
  int res;
  res = 10;
  if (A == a)
    res = a;
  if (B == a)
    res = a;
  if (C == a)
    res = a;
  if (D == a)
    res = a;
  return res;
}
