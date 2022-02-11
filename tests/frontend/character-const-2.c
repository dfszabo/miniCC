// RUN: AArch64

// FUNC-DECL: int test(unsigned)
// TEST-CASE: test(0) -> 116
// TEST-CASE: test(1) -> 101
// TEST-CASE: test(2) -> 115
// TEST-CASE: test(3) -> 116
// TEST-CASE: test(4) -> 0

char str[5] = {'t', 'e', 's', 't', '\0'};

unsigned test(unsigned i) {
  return str[i];
}
