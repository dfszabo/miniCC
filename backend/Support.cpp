#include "Support.hpp"

uint64_t GetNextAlignedValue(unsigned Val, unsigned Alignment) {
  return (Val + Alignment - 1) & ~(Alignment - 1);
}

bool IsInt(uint64_t Number, unsigned BitWidth) {
  int64_t S = ((int64_t)Number) >> BitWidth;
  return S == 0 || S == -1;
}
