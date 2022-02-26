// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 1


void test_deref(int **ptr) {
  **ptr = 1;
}

int test() {
  int a = 0;
  int *a_ptr = &a;
  test_deref(&a_ptr);
  return a;
}
