// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test(0) -> 1
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(2) -> 2
// TEST-CASE: test(3) -> 3
// TEST-CASE: test(4) -> 5


int test_arr(int *ptr) {
  return *ptr;
}

int test(int idx) {
  int arr[5] = {1, 1, 2, 3, 5};
  return test_arr(&arr[idx]);
}
