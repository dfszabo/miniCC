// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 1
// TEST-CASE: test(1) -> 2
// TEST-CASE: test(2) -> 3
// TEST-CASE: test(3) -> 4
// TEST-CASE: test(4) -> 5


int get_element(int *a, int idx) {
  return a[idx];
}

int test(int idx) {
  int arr[5] = {1, 2, 3, 4, 5};

  return get_element(arr, idx);
}
