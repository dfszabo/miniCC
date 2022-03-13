// COMPILE-FAIL
// EXTRA-FLAGS: -Wall


int test(int a) {
  switch (a) {
  case 1:
    return 1;
  default:
    return 2;
  default:
    return 3;
  }
}
