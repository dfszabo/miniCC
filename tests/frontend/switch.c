// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 10
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(2) -> 7
// TEST-CASE: test(3) -> 4
// TEST-CASE: test(4) -> 5
// TEST-CASE: test(5) -> 10

enum { A, B, C };

int test(int a) {
  int res;

  switch (a) {
  case B: res = a; break;
  case C: res = 7; break;
  case 3:
  case 4:
    res = 1 + a;
    break;
  default: res = 10; break;
  }
  return res;
}
