// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 111

int test2(int *a) { return *a; }

int test() {
  int a = 111;
  return test2(&a);
}
