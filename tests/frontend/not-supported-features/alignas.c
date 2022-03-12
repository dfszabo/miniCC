// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


struct S {
  float a;
};

int test(int a) {
  _Alignas(32) struct S a;
  return 1;
}
