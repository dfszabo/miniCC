// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 1

#include <stdio.h>

int test() {
  char str[512];
  unsigned str_length = sprintf(str, __FILE__);

  if (str_length < 6)
    return 0;

  if (str[str_length - 6] != 'f')
    return 0;
  if (str[str_length - 5] != 'i')
    return 0;
  if (str[str_length - 4] != 'l')
    return 0;
  if (str[str_length - 3] != 'e')
    return 0;
  if (str[str_length - 2] != '.')
    return 0;
  if (str[str_length - 1] != 'c')
    return 0;

  return 1;
}
