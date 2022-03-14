// COMPILE-TEST
// EXTRA-FLAGS: -dump-ir

void test();

// CHECK: .loop_header0:
// CHECK: 	call	test()
// CHECK: 	j	<loop_header0>
void test_fp_true() {
  while (1.0)
    test();
}

// CHECK: .entry_test_fp_false:
// CHECK: 	ret	
void test_fp_false() {
  while (0.0)
    test();
}
