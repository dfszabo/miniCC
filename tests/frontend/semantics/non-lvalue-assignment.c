// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL


int test(int a) {
  0 = a;
  0 += a;
  0 -= a;
  0 *= a;
  0 /= a;
  0 %= a;
  0 &= a;
  0 |= a;
  0 ^= a;
  0 <<= a;
  0 >>= a;
  return a;
}
