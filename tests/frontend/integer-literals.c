// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 1


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

int test() {
  if (test_u() != 13u)
    return 0;
  if (test_l() != 13l)
    return 0;
  if (test_ul() != 13ul)
    return 0;
  if (test_ll() != 13ll)
    return 0;
  if (test_ull() != 13ull)
    return 0;
  return 1;
}
