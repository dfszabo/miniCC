// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 246

struct P {
  int x;
  int y;
  int z;
};

struct P GlobalStruct;

int test() {
  GlobalStruct.x = 123;
  GlobalStruct.z = 123;
  return GlobalStruct.x + GlobalStruct.z;
}
