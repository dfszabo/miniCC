int gcd(int a, int b) {
  int R;

  while ((a % b) > 0) {
    R = a % b;
    a = b;
    b = R;
  }

  return b;
}
