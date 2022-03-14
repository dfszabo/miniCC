// COMPILE-TEST
// EXTRA-FLAGS: -dump-ir


// CHECK: .entry_test_fp:
// CHECK: 	ret	0<u32>
int test_fp() {
  return ((1.0 > 0.5) != (1.0 == 1.0) | (1.0 < 0.5) >= (1.0 <= 1.0));
}

// CHECK: .entry_test_int:
// CHECK: 	ret	0<u32>
int test_int() {
  return ((1 > 0) != (1 == 1) | (1 < 0) >= (1 <= 1));
}
