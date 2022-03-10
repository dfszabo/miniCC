// RUN: AArch64

// EXTRA-FLAGS: -Wall
// FUNC-DECL: int test(int)
// COMPILE-FAIL

int test(int a) {
  switch (a) {
  case a:
    return 1;
  }
}
