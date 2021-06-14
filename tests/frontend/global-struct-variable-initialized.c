// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 4

struct P {
  int x;
  int y;
  int z;
};

struct P GlobalStruct = {1 , 2, 3};

int test() {
  return GlobalStruct.x + GlobalStruct.z;
}
