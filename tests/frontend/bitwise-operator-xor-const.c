// RUN: AArch64

// FUNC-DECL: int test_xor(unsigned)
// TEST-CASE: test_xor(1) -> 14
// TEST-CASE: test_xor(255) -> 240
// TEST-CASE: test_xor(7) -> 8


unsigned test_xor(unsigned a) {
  return a ^ 15;
}
