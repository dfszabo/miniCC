// COMPILE-FAIL

int func(int a);

int test() {
  return func(1, 2);
}
