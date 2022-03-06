// RUN: AArch64

// FUNC-DECL: long test()
// TEST-CASE: test() -> -1


long test() {
  int *res = -1;
  return res;
}
