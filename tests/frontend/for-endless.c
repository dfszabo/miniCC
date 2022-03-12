// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(1) -> 1
// TEST-CASE: test(10) -> 10


int test(int a) {
  int i = 0;
  for (;;) {
    if (a == i)
      break;
    i++;
  }

  return i;
}
