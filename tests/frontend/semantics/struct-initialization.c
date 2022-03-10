// RUN: AArch64

// FUNC-DECL: int test()
// COMPILE-FAIL

struct Point {
  int x;
  int y;
  int z;
};


int test() {
  struct Point P = (struct Point) { .y = 1, .z = 2, .w = 3};
  return 1;
}
