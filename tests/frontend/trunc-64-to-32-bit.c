// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 111


int test() {
  long long a = 111 + (1ll << 32);
  return a;
}
