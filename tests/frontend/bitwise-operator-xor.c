// RUN: AArch64

// FUNC-DECL: int test_xor(unsigned, unsigned)
// TEST-CASE: test_xor(1, 0) -> 1
// TEST-CASE: test_xor(1, 1) -> 0
// TEST-CASE: test_xor(123, 123) -> 0
// TEST-CASE: test_xor(7, 8) -> 15


unsigned test_xor(unsigned a, unsigned b) {
  return a ^ b;
}
