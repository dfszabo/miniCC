// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 1
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(2) -> 2
// TEST-CASE: test(3) -> 3
// TEST-CASE: test(4) -> 5


int test_arr(int arr[], int i) { return arr[i]; }

int test(int i) {
  int arr[5] = {1, 1, 2, 3, 5};
  return test_arr(arr, i);
}
