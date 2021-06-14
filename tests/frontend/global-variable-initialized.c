// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 123

int global = 123;

int test() {
  return global;
}
