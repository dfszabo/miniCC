// RUN: AArch64

// FUNC-DECL: short test_short(unsigned short)
// TEST-CASE: test_short((1 << 16) - 1) -> -1
// TEST-CASE: test_short(32768) -> -32768
// TEST-CASE: test_short(-1) -> -1

short test_short(unsigned short c) {
   return c;
}
