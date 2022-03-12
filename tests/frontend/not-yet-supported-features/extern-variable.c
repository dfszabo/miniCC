// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL

extern int b;

int test(int a) {
  return b + a;
}
