// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 1


void test_deref(int *ptr) {
  *ptr = 1;
}

int test() {
  int a = 0;
  test_deref(&a);
  return a;
}
