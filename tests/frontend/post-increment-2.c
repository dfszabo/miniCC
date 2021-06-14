// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 1
// TEST-CASE: test(1) -> 3
// TEST-CASE: test(2) -> 5

struct Point {
  int x;
  int y;
  int z;
};

int test(int a) {
  struct Point P;
  P.y = a;
  P.y++;
  return P.y + a;
}
