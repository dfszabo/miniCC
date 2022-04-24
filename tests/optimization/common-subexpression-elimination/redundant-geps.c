// COMPILE-TEST
// EXTRA-FLAGS: -cse -dump-ir

struct S {
  int a;
} s;

// The IR without optimization
//
//	gep	$0<*i32>, @s<struct.S>, 0<u32>
//	ld	$1<i32>, [$0<*i32>]
//	gep	$2<*i32>, @s<struct.S>, 0<u32>
//	ld	$3<i32>, [$2<*i32>]
//	add	$4<i32>, $1<i32>, $3<i32>
//	gep	$5<*i32>, @s<struct.S>, 0<u32>
//	ld	$6<i32>, [$5<*i32>]
//	add	$7<i32>, $4<i32>, $6<i32>
//	ret	$7<i32>

// The optimization was able to get rid of 4 instruction: 2 gep, 2 ld

// CHECK:	gep	$0<*i32>, @s<struct.S>, 0<u32>
// CHECK:	ld	$1<i32>, [$0<*i32>]
// CHECK:	add	$4<i32>, $1<i32>, $1<i32>
// CHECK:	add	$7<i32>, $4<i32>, $1<i32>
// CHECK:	ret	$7<i32>
int test() {
  return s.a + s.a + s.a;
}
