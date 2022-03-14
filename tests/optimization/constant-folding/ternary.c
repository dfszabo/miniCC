// COMPILE-TEST
// EXTRA-FLAGS: -dump-ir


// CHECK: .entry_test_true:
// CHECK: 	ret	1<u32>
int test_true() {
  return 1 ? 1 : 2;
}

// CHECK: .entry_test_false:
// CHECK: 	ret	2<u32>
int test_false() {
  return 0 ? 1 : 2;
}
