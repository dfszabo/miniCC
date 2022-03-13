// COMPILE-FAIL

int test(int a) {
  switch () {
  case 1:
    break;
  default:
    break;
  }
}
