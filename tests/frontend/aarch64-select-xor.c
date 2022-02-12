// RUN: AArch64

// FUNC-DECL: unsigned test(unsigned,unsigned)
// TEST-CASE: test(1, 1) -> 0
// TEST-CASE: test(15, 1) -> 14
// TEST-CASE: test(255, 240) -> 15
// TEST-CASE: test(15, 60) -> 51
// TEST-CASE: test(12345, 65535) -> 53190


unsigned test(unsigned a, unsigned b) {
  return a ^ b;
}
