// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 3

int calle(int a) {
  return 1 + a;
}

int test() {
  return calle(2);
}
