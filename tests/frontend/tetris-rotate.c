// RUN: AArch64
// FUNC-DECL: unsigned rotate(unsigned, unsigned, unsigned)
// TEST-CASE: rotate(2, 3, 0) -> 11
// TEST-CASE: rotate(2, 3, 1) -> 2
// TEST-CASE: rotate(2, 3, 2) -> 4
// TEST-CASE: rotate(2, 3, 3) -> 13
// TEST-CASE: rotate(2, 3, 7) -> 13


typedef unsigned int uint;

uint rotate(uint x, uint y, uint rotation) {
  uint rotated_idx = 0;

  switch (rotation % 4) {
  case 0:
    rotated_idx = x * 4 + y;
    break;
  case 1:
    rotated_idx = 12 + x - (y * 4);
    break;
  case 2:
    rotated_idx = 15 - (x * 4) - y;
    break;
  case 3:
    rotated_idx = 3 - x + (y * 4);
    break;
  default:
    break;
  }

  return rotated_idx;
}
