// COMPILE-TEST
// EXTRA-FLAGS: -dump-ir


// CHECK: ret	47
int test_int() {
  return 1 + 2 * 3 - 10 / 2 % 3 & 3 | 10 << 2 ^ 15 >> 1;
}

// CHECK: ret	2.000000
double test_fp() {
  return 1.0 + 2.0 * 3.0 - 10.0 / 2.0;
}
