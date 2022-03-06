// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(1) -> 1

// TODO: Add negative test to show that assertion really happens.
// This is not implemented yet in the test system.

#include <assert.h>

int test(int a) {
  assert(a > 0);
  return a;
}
