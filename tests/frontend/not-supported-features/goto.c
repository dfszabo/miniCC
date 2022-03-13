// COMPILE-FAIL

int test( int a) {
  goto end;
end:
  return a;
}
