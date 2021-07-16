// RUN: AArch64
// FUNC-DECL: int test(int)
// TEST-CASE: test(1) -> 2

typedef enum { FIRST, SECOND } Enum;

int test(Enum a) {
  return a + 1;
}
