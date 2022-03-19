// COMPILE-TEST
// EXTRA-FLAGS: -cse -dump-ir

// Note that $6 is reused for load
// Without optimization the end would look like this
//	str	[$7<*i32>], 3<u32>
//	gep	$8<*i32>, $4<*[3 x i32]>, 1<u32>
//	ld	$9<i32>, [$8<*i32>]
//	gep	$10<*i32>, $4<*[3 x i32]>, 1<u32>
//	ld	$11<i32>, [$10<*i32>]
//	add	$12<i32>, $9<i32>, $11<i32>
//	ret	$12<i32>
//
// The optimization was able to get rid of 3 instruction: 2 gep, 1 ld

// CHECK: 	sa	$0<*i32>
// CHECK: 	sa	$2<*i32>
// CHECK: 	sa	$4<*[3 x i32]>
// CHECK: 	str	[$0<*i32>], $a
// CHECK: 	str	[$2<*i32>], $b
// CHECK: 	gep	$5<*i32>, $4<*[3 x i32]>, 0<u32>
// CHECK: 	str	[$5<*i32>], 1<u32>
// CHECK: 	gep	$6<*i32>, $4<*[3 x i32]>, 1<u32>
// CHECK: 	str	[$6<*i32>], 2<u32>
// CHECK: 	gep	$7<*i32>, $4<*[3 x i32]>, 2<u32>
// CHECK: 	str	[$7<*i32>], 3<u32>
// CHECK: 	ld	$9<i32>, [$6<*i32>]
// CHECK: 	add	$12<i32>, $9<i32>, $9<i32>
// CHECK: 	ret	$12<i32>
int test(int a, int b) {
  int arr[3] = {1 ,2, 3};
  return arr[1] + arr[1];
}
