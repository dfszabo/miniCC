// RUN: AArch64

// FUNC-DECL: unsigned test(unsigned,unsigned)
// TEST-CASE: test(1, 1) -> 1
// TEST-CASE: test(15, 1) -> 1
// TEST-CASE: test(255, 240) -> 240
// TEST-CASE: test(15, 60) -> 12
// TEST-CASE: test(12345, 65535) -> 12345


unsigned test(unsigned a, unsigned b) {
  return a & b;
}
