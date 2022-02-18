// RUN: AArch64

// FUNC-DECL: unsigned test(int)
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(12) -> 2
// TEST-CASE: test(123) -> 3
// TEST-CASE: test(1234) -> 4
// TEST-CASE: test(12345) -> 5

int sprintf(char *dst, const char *fmt, ...);

int test(int num) {
  char str[10];
  return sprintf(str, "%d", num);
}
