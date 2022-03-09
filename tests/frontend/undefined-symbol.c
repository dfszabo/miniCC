// RUN: AArch64

// FUNC-DECL: int test()
// COMPILE-FAIL

int test() {
  return undefined_var;
}
