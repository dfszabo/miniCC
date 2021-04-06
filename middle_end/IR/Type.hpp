#ifndef IRTYPE_HPP
#define IRTYPE_HPP

#include <cassert>
#include <cstdint>
#include <string>

class IRType {
public:
  enum TKind : uint8_t { INVALID, NONE, FP, UINT, SINT, PTR };

  IRType() : Kind(INVALID), BitWidth(0) {}

  IRType(IRType::TKind kind) : Kind(kind), BitWidth(32) {}

  IRType(IRType::TKind kind, uint8_t BW) : Kind(kind), BitWidth(BW) {}

  IRType(IRType::TKind kind, uint8_t BW, unsigned NumberOfElements)
      : Kind(kind), BitWidth(BW), NumberOfElements(NumberOfElements) {}

  void SetKind(IRType::TKind K) { Kind = K; }

  void SetToPointerKind() { Kind = PTR; }

  uint8_t GetPointerLevel() const { return PointerLevel; }
  void SetPointerLevel(uint8_t pl) {
    assert(pl < 10 && "Unrealistic pointer level");
    PointerLevel = pl;
  }

  static IRType CreateBool() { return IRType(SINT, 1); }
  static IRType CreateFloat(uint8_t BitWidht = 32) {
    return IRType(FP, BitWidht);
  }

  static IRType CreateInt(uint8_t BitWidht = 32) {
    return IRType(SINT, BitWidht);
  }

  bool operator==(const IRType &RHS) {
    return BitWidth == RHS.BitWidth && Kind == RHS.Kind;
  }

  bool IsFP() const { return Kind == FP; }
  bool IsINT() const { return Kind == SINT || Kind == UINT; }
  bool IsPTR() const { return PointerLevel > 0; }
  bool IsVoid() const { return Kind == NONE; }

  void SetNumberOfElements(unsigned N) { NumberOfElements = N; }
  size_t GetBitSize() const { return BitWidth; }

  size_t GetByteSize() const;

  IRType GetBaseType() const { return IRType(Kind, BitWidth); }

  std::string AsString() const;

private:
  uint8_t BitWidth;
  uint8_t PointerLevel = 0;
  unsigned NumberOfElements = 1;
  TKind Kind;
};

#endif
