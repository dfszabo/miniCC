// RUN: AArch64

// FUNC-DECL: int test(int, long long)
// TEST-CASE: test(0, (1ll << 8) + 1) -> 1
// TEST-CASE: test(1, (1ll << 8) - 1) -> -1
// TEST-CASE: test(2, (1ll << 16) + 256) -> 256
// TEST-CASE: test(3, (1ll << 16) - 1) -> -1
// TEST-CASE: test(4, (1ll << 32) + 0x10000) -> 0x10000
// TEST-CASE: test(5, (1ll << 32) - 1) -> -1


int test(int select, long long i) {
  int res = 0;

  if (select == 0)
    res = (unsigned char)i;
  else if (select == 1)
    res = (char)i;
  else if (select == 2)
    res = (unsigned short)i;
  else if (select == 3)
    res = (short)i;
  else if (select == 4)
    res = (unsigned)i;
  else
    res = (int)i;

  return res;
}
