// COMPILE-TEST
// EXTRA-FLAGS: -arch=aarch64 -dump-ir

// CHECK: memcopy
int test(int i) {
  int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  return arr[i];
}
