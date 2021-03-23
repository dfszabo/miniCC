int foo() {
  int i;

  for (i = 0; i < 100; i = i + 1) {
    i = i + 1;
  }

  return i;
}
