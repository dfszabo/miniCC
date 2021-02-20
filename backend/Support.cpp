#include "Support.hpp"

uint64_t GetNextAlignedValue(unsigned Val, unsigned Alignment) {
  return (Val + Alignment - 1) & ~(Alignment - 1);
}
