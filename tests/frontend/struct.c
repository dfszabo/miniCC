// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 3

struct Point {
  int x;
  int y;
  int z;
};

int struct_param(struct Point P) {
  return P.x + P.z;
}

struct Point struct_return() {
  struct Point P;
  P.x = 1;
  P.z = 2;
  return P;
}

int test() {
  struct Point P;
  P = struct_return();
  return struct_param(P);
}
