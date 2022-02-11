// RUN: AArch64

// FUNC-DECL: unsigned test()
// TEST-CASE: test() -> 15728655

unsigned test() {
  return 0x00f0000f;
}
