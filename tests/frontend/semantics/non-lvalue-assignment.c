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
