// RUN: AArch64
// FUNC-DECL: int gcd(int, int)
// TEST-CASE: gcd(12, 30) -> 6
// TEST-CASE: gcd(12, 20) -> 4

int gcd(int a, int b) {
  int R;

  while ((a % b) > 0) {
    R = a % b;
    a = b;
    b = R;
  }

  return b;
}
