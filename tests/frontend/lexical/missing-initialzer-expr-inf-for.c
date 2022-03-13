// COMPILE-FAIL

int test(int a) {
  for (int i = ;;)
    return 1;
}
