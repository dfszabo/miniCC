// RUN: AArch64
// FUNC-DECL: unsigned int test(unsigned char)
// TEST-CASE: test(0) -> 0
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(2) -> 4

unsigned int test(unsigned char c) {
  return c * c;
}
