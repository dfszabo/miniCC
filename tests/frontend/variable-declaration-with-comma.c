// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(23) -> 23


int test(int param) {
  int *ptr, val;
  val = param;
  ptr = &val;
  return *ptr; 
}
