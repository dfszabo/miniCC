// RUN: AArch64

// FUNC-DECL: int test(float,float)
// TEST-CASE: test(0, 1.0) -> 1 + -1 + 0 + 0
// TEST-CASE: test(0.5, 0.5) -> 1 + 0 + 0 + 1
// TEST-CASE: test(0.8, 0.3) -> 1 + 0 + 0 + 2
// TEST-CASE: test(-1.3, -1.8) -> -3 + 0 + 2 + 0


int test_add(float a, float b) { return a + b; }
int test_sub(float a, float b) { return a - b; }
int test_mul(float a, float b) { return a * b; }
int test_div(float a, float b) { return a / b; }

int test(float a, float b) {
  int res = 0;

  res += test_add(a, b);
  res += test_sub(a, b);
  res += test_mul(a, b);
  res += test_div(a, b);

  return res;
}
