// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 3

// To test that it will work when the type is not a typedef, but
// is in a form of "struct <ID>"

struct Point {
  int x;
  int y;
  int z;
};


int test() {
  struct Point P = (struct Point) { .y = 1, .z = 2, .z = 3};
  return P.z;
}
