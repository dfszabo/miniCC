#ifndef SUPPORT_HPP
#define SUPPORT_HPP

#include <cstdint>

template <unsigned BitWidth> bool IsInt(uint64_t Number) {
  int64_t S = ((int64_t)Number) >> BitWidth;
  return S == 0 || S == -1;
}

template <unsigned BitWidth> bool IsUInt(uint64_t Number) {
  int64_t S = ((int64_t)Number) >> BitWidth;
  return S == 0;
}

// Only use with power of 2 alignments
// FIXME: make it more general and safe
uint64_t GetNextAlignedValue(unsigned Val, unsigned Alignment);

#endif
