// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 23


int test() {
  int *res;
  res = 23;
  return res;
}
