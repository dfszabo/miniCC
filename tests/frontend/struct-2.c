// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 3

// Testing the right handling of structure params and return values when they
// cannot be passed by value because of their size

struct Point {
  int x[5];
};

int struct_param(struct Point P) {
  return P.x[0] + P.x[4];
}

struct Point struct_return() {
  struct Point P;
  P.x[0] = 1;
  P.x[4] = 2;
  return P;
}

int test() {
  struct Point P;
  P = struct_return();
  return struct_param(P);
}
