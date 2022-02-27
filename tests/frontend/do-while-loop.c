// RUN: AArch64

// FUNC-DECL: int test(int,int)
// TEST-CASE: test(5, 0) -> 5
// TEST-CASE: test(5, 1) -> 10
// TEST-CASE: test(5, 10) -> 55

int test(int n, int stop) {
  int result = 0;
  int i = 0;

  do {
    result += n;
    i++;
  } while (i <= stop);

  return result;
}
