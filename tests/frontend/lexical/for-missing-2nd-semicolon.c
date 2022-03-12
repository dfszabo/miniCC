// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


int test(int a) {
  for (int i = 1;)
    return 1;
}
