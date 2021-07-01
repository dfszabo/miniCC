// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 7

int global[3][4] = {{1,  2,  3,  4},
                    {5,  6,  7,  8},
                    {9, 10, 11, 12}};

int test() {
  return global[1][2];
}
