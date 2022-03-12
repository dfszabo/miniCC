// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL

struct {
  int a;
} o;

int test(int a) {
  return 1;
}
