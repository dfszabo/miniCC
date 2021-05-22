// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 111

int test() {
  int arr[10];
  arr[6] = 111;
  return arr[6];
}
