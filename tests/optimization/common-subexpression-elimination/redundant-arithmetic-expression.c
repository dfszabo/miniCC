// COMPILE-TEST
// EXTRA-FLAGS: -cse -dump-ir

// Note that only one "add" instruction present in the output instead of 3

// CHECK: 	sa	$0<*i32>
// CHECK: 	sa	$2<*i32>
// CHECK: 	str	[$0<*i32>], $a
// CHECK: 	str	[$2<*i32>], $b
// CHECK: 	ld	$4<i32>, [$0<*i32>]
// CHECK: 	ld	$5<i32>, [$2<*i32>]
// CHECK: 	add	$6<i32>, $4<i32>, $5<i32>
// CHECK: 	mul	$10<i32>, $6<i32>, $6<i32>
// CHECK: 	sub	$14<i32>, $10<i32>, $6<i32>
// CHECK: 	ret	$14<i32>
int test(int a, int b) {
  return (a + b) * (a + b) - (a + b);
}
