// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 123

int global[10];

int test() {
  global[4] = 123;
  return global[4];
}
