// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(23) -> 23

int *ptr, val;

int test(int param) {
  val = param;
  ptr = &val;
  return *ptr; 
}
