// RUN: AArch64

// FUNC-DECL: int test(unsigned)
// TEST-CASE: test(0) -> 'H'
// TEST-CASE: test(4) -> 'o'
// TEST-CASE: test(6) -> 'W'
// TEST-CASE: test(9) -> 'l'
// TEST-CASE: test(11) -> '\0'


int test(unsigned i) {
  char *str = "Hello World";
  return str[i];
}
