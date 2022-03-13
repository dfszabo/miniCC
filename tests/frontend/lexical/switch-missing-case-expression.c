// COMPILE-FAIL

int test(int a) {
  switch (a) {
  case :
    break;
  default:
    break;
  }
}
