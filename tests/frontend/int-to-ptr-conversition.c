// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 1


struct node {
  int data;
} * StructData;

int test() {
  return (StructData == 0);
}
