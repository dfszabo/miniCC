// COMPILE-FAIL

int test(int a) {
  switch (a) {
  case a:
    return 1;
  }
}
