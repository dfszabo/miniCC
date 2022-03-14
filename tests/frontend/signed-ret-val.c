// COMPILE-TEST
// EXTRA-FLAGS: -dump-ir


// CHECK: func test ($a :i32) -> i32:
// CHECK: .entry_test:
// CHECK: 	sa	$0<*i32>
// CHECK: 	str	[$0<*i32>], $a
// CHECK: 	ld	$2<i32>, [$0<*i32>]
// CHECK: 	ret	$2<i32>
signed int test(int a) {
  return a;
}
