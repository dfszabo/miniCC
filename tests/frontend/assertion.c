// RUN-FAIL: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 0


#include <assert.h>

int test(int a) {
  assert(a > 0);
  return a;
}
