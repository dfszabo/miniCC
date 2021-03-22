int test_ra(int a, int b, int c, int d) {
  int res;

  res = a + b;
  res = res - b;
  res = res * c;
  res = res / d;
  res = res * a;

  return res;
}
