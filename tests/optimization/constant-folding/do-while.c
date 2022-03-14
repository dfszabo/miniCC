// COMPILE-TEST
// EXTRA-FLAGS: -dump-ir

void test();

// CHECK: .entry_test_true:
// CHECK: 	j	<loop_body0>
// CHECK: .loop_body0:
// CHECK: 	call	test()
// CHECK: 	j	<loop_body0>
// CHECK: 	ret	
void test_true() {
  do
    test();
  while (1);
}

// CHECK: .entry_test_false:
// CHECK: 	j	<loop_body0>
// CHECK: .loop_body0:
// CHECK: 	call	test()
// CHECK: 	ret	
void test_false() {
  do
    test();
  while (0);
}
