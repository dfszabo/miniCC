int test_single_ptr(int *a) {
  return *a + 1;
}

int test_double_ptr(int **a) {
  return **a + 1;
}

char test_single_ptr_char(char *a) {
  return *a + 1;
}
