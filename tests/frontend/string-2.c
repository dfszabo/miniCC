// RUN: AArch64

// FUNC-DECL: int test(unsigned)
// TEST-CASE: test(0) -> 'H'
// TEST-CASE: test(4) -> 'o'
// TEST-CASE: test(6) -> 'W'
// TEST-CASE: test(9) -> 'l'
// TEST-CASE: test(11) -> '\0'

char *str = "Hello World";

int test(unsigned i) {
  return str[i];
}
