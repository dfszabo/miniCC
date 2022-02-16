// RUN: AArch64

// FUNC-DECL: int test(int, int)
// TEST-CASE: test(0, 0) -> 0
// TEST-CASE: test(1, 0) -> 1
// TEST-CASE: test(0, 1) -> 1
// TEST-CASE: test(1, 1) -> 1

int test(int a, int b) {
  if (a || b)
    return 1;
  return 0;
}
