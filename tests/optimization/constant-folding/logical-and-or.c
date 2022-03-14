// COMPILE-TEST
// EXTRA-FLAGS: -dump-ir

void test();
int a;

// CHECK: .entry_test_and_always_false:
// CHECK: 	ret	
void test_and_always_false() {
  if (0 && a)
    test();
}

// CHECK: .entry_test_and_check_rhs_only:
// CHECK: 	ld	$1<i32>, [@a<i32>]
// CHECK: 	cmp.eq	$2<i1>, $1<i32>, 0<u32>
// CHECK: 	br	$2<i1>, <if_end0>
// CHECK: .if_true0:
// CHECK: 	call	test()
// CHECK: 	j	<if_end0>
// CHECK: .if_end0:
// CHECK: 	ret	
void test_and_check_rhs_only() {
  if (1 && a)
    test();
}

// CHECK: .entry_test_or_check_rhs_only:
// CHECK: 	ld	$1<i32>, [@a<i32>]
// CHECK: 	cmp.eq	$2<i1>, $1<i32>, 0<u32>
// CHECK: 	br	$2<i1>, <if_end0>
// CHECK: .if_true0:
// CHECK: 	call	test()
// CHECK: 	j	<if_end0>
// CHECK: .if_end0:
// CHECK: 	ret	
void test_or_check_rhs_only() {
  if (0 || a)
    test();
}

// CHECK: .entry_test_and_always_true:
// CHECK: 	call	test()
// CHECK: 	ret
void test_and_always_true() {
  if (1 || a)
    test();
}
