// RUN: AArch64
// FUNC-DECL: long test(unsigned long long)
// TEST-CASE: test(0) -> 0
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(-1) -> -1

long test(unsigned long long c) {
   return c;
}
