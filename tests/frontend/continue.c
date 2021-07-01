// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(5) -> 20
// TEST-CASE: test(20) -> 5

int test(int num) {
  int i;
  int counter = 0;
  for (i = 1; i < 101; i++) {
    if (i % num != 0)
      continue;
    counter++;
  }
  return counter;
}