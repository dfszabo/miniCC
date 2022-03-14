// COMPILE-TEST
// EXTRA-FLAGS: -dump-ir

void test();

// CHECK: .entry_test_true:
// CHECK: 	call	test()
// CHECK: 	ret
void test_true() {
  if (1)
    test();
}

// CHECK: .entry_test_false:
// CHECK: 	ret	
void test_false() {
  if (0)
    test();
}
