// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 1

float callee() { return 1; }

int test() { return callee(); }
