// RUN: AArch64

// FUNC-DECL: int test(int)
// TEST-CASE: test(0) -> 12
// TEST-CASE: test(1) -> 14
// TEST-CASE: test(123) -> 9

int test(int a) {
    unsigned res = __LINE__;

    if (a == 0)
      res = __LINE__;
    else if (a == 1)
      res = __LINE__;

    return res;
}
