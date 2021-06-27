// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 111

int test() {
  unsigned char arr[5][5];
  arr[2][3] = 111;
  return arr[2][3];
}
