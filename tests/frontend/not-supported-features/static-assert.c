// COMPILE-FAIL

int test(int a) {
  _Static_assert(3 == 3, "test");
}
