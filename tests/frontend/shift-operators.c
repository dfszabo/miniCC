// RUN: AArch64

// FUNC-DECL: int test_left_shift(unsigned, unsigned)
// TEST-CASE: test_left_shift(1, 0) -> 1
// TEST-CASE: test_left_shift(1, 1) -> 2
// TEST-CASE: test_left_shift(1, 10) -> 1024

// FUNC-DECL: int test_right_shift(unsigned, unsigned)
// TEST-CASE: test_right_shift(1024, 0) -> 1024
// TEST-CASE: test_right_shift(1024, 1) -> 512
// TEST-CASE: test_right_shift(1024, 10) -> 1


unsigned test_left_shift(unsigned a, unsigned amt) {
  return a << amt;
}

unsigned test_right_shift(unsigned a, unsigned amt) {
  return a >> amt;
}