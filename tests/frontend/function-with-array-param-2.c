// TODO: enable test once feature is supported

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(0, 0) -> 1
// TEST-CASE: test(0, 1) -> 1
// TEST-CASE: test(0, 2) -> 2
// TEST-CASE: test(0, 3) -> 3
// TEST-CASE: test(0, 4) -> 5
// TEST-CASE: test(1, 0) -> 10
// TEST-CASE: test(1, 1) -> 10
// TEST-CASE: test(1, 2) -> 20
// TEST-CASE: test(1, 3) -> 30
// TEST-CASE: test(1, 4) -> 50


int test_arr(int arr[][5], int i, int j) { return arr[i][j]; }

int test(int i, int j) {
  int arr[2][5] = {{1, 1, 2, 3, 5}, {10, 10, 20, 30, 50}};
  return test_arr(arr, i, j);
}
