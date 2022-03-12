// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL

int func(int a) { return a * 2; }

int (*fun_ptr)(int) = &func;

int test(int a) {
  return (*fun_ptr)(10 + a);
}
