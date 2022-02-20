// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 2
// TEST-CASE: test(1) -> 3
// TEST-CASE: test(2) -> 5
// TEST-CASE: test(3) -> 7
// TEST-CASE: test(4) -> 11

int test(int i) {
  int a[] = {2, 3, 5, 7, 11};
  return a[i];
}
