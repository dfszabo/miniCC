// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 123

int global[5] = { 1, 2, 3, 123, 5};

int test() {
  return global[3];
}
