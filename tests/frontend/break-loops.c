// RUN: AArch64

// FUNC-DECL: int test_for()
// TEST-CASE: test_for() -> 50

// FUNC-DECL: int test_while()
// TEST-CASE: test_while() -> 49

int test_for() {
  int i;
  for (i = 0; i < 100; i++)
    if (i == 50)
      break;
  return i;
}

int test_while() {
  int i = 0;
  while (i < 100) {
    if (i == 49)
      break;
    i++;
  }
  return i;
}
