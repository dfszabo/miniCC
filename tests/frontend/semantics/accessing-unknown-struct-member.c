// COMPILE-FAIL

struct S { int y; };

int test(int a) {
  struct S s;
  return s.x;
}
