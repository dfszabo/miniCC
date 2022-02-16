// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 124


int test() {
  unsigned char u8;
  char i8;
  unsigned short u16;
  short i16;
  unsigned int u32;
  int i32;
  unsigned long u32l;
  long i32l;
  unsigned long long u64;
  long long i64;
  int *i32_ptr;
  char *i8_ptr;

  unsigned res = 0;

  res = sizeof(unsigned char);
  res += sizeof(u8);
  res += sizeof(char);
  res += sizeof(i8);

  res += sizeof(unsigned short);
  res += sizeof(u16);
  res += sizeof(short);
  res += sizeof(i16);

  res += sizeof(unsigned int);
  res += sizeof(u32);
  res += sizeof(int);
  res += sizeof(i32);

  res += sizeof(unsigned long);
  res += sizeof(u32l);
  res += sizeof(long);
  res += sizeof(i32l);

  res += sizeof(unsigned long long);
  res += sizeof(u64);
  res += sizeof(long long);
  res += sizeof(i64);

  res += sizeof(char*);
  res += sizeof(i8_ptr);
  res += sizeof(int*);
  res += sizeof(i32_ptr);

  return res;
}
