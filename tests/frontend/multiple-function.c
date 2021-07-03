// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 3

int foo() {
  return 1;
}

int bar() {
  return 2;
}

int test() {
  return foo() + bar();
}
