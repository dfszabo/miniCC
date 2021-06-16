// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 1
// TEST-CASE: test(1) -> 2

// TODO: test other operators like *= when mul and div fixed
int test(int a) {
  a += 2;
  a -= 1;
  return a;
}
