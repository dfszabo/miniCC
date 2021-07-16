// RUN: AArch64

// FUNC-DECL: unsigned test_u()
// TEST-CASE: test_u() -> 13

// FUNC-DECL: long test_l()
// TEST-CASE: test_l() -> 13

// FUNC-DECL: unsigned long test_ul()
// TEST-CASE: test_ul() -> 13

// FUNC-DECL: long long test_ll()
// TEST-CASE: test_ll() -> 13

// FUNC-DECL: unsigned long long test_ull()
// TEST-CASE: test_ull() -> 13


unsigned test_u() {
  return 13u;
}

long test_l() {
  return 13l;
}

unsigned long test_ul() {
  return 13ul;
}

long long test_ll() {
  return 13ll;
}

unsigned long long test_ull() {
  return 13ull;
}
