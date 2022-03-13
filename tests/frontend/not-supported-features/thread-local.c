// COMPILE-FAIL

int test(int a) {
  _Thread_local int var = 5;
  return 1;
}
