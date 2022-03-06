// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(12) -> 2
// TEST-CASE: test(123) -> 3
// TEST-CASE: test(1234) -> 4
// TEST-CASE: test(12345) -> 5

#include <stdio.h>

int test(int num) {
  char str[10];
  return sprintf(str, "%d", num);
}
