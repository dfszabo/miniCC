// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 100

int test() {
  int i;

  for (i = 0; i < 100; i = i + 1) {
    i = i + 1;
  }

  return i;
}
