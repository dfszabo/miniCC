// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(5, 10) -> 50
// TEST-CASE: test(10, 10) -> 100
// TEST-CASE: test(8, 8) -> 64


int test(int addend, int stop) {
  int res = 0;
  for (int i = 0, b = addend; i < stop; i++)
    res += b;

  return res;
}
