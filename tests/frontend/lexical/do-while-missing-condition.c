// COMPILE-FAIL

int test(int a) {
  do {
    return a;
  } while ();
}
