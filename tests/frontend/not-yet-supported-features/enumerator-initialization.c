// COMPILE-FAIL

enum { A = 2, B = 1};

int test(int a) {
  return a + A;
}
