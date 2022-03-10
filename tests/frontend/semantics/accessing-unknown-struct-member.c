// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL

struct S { int y; };

int test(int a) {
  struct S s;
  return s.x;
}
