// COMPILE-FAIL

int test(int a) {
  for (int i = 1;)
    return 1;
}
