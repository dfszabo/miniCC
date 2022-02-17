// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 6

typedef struct P {
  int x;
  int y;
  int z;
} Point;

int cast(int* s) {
  Point *point = (Point*)s;
  return point->x + point->y + point->z;
}

int test() {
  Point p = {1, 2, 3};
  return cast(&p);
}
