// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL

struct A { int a; };

int test(int b) {
  struct A a;
  a = b;
  return b;
}
