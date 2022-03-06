// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 1

int g = 0;

int test() {
  ++g;
  return g;
}
