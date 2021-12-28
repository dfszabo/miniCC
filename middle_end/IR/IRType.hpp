#ifndef IRTYPE_HPP
#define IRTYPE_HPP

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

class IRType {
public:
  enum TKind : uint8_t { INVALID, NONE, FP, UINT, SINT, PTR, STRUCT };

  IRType() : Kind(INVALID), BitWidth(0) {}

  IRType(IRType::TKind kind) : Kind(kind), BitWidth(32) {}

  IRType(IRType::TKind kind, uint8_t BW) : Kind(kind), BitWidth(BW) {}

  void SetKind(IRType::TKind K) { Kind = K; }

  void SetToPointerKind() { Kind = PTR; }

  uint8_t GetPointerLevel() const { return PointerLevel; }
  void SetPointerLevel(uint8_t pl) {
    assert(pl < 10 && "Unrealistic pointer level");
    PointerLevel = pl;
  }
  void IncrementPointerLevel() { PointerLevel++; }
  void DecrementPointerLevel() {
    assert(PointerLevel > 0 && "Cannot decrement below 0");
    PointerLevel--;
  }

  void ReduceDimension();

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

  bool IsInvalid() const { return Kind == INVALID; }
  bool IsFP() const { return Kind == FP; }
  bool IsINT() const { return Kind == SINT || Kind == UINT; }
  bool IsPTR() const { return PointerLevel > 0; }
  bool IsStruct() const { return Kind == STRUCT; }
  bool IsArray() const { return !Dimensions.empty(); }
  bool IsVoid() const { return Kind == NONE; }

  void SetDimensions(const std::vector<unsigned>& N) { Dimensions = N; }
  std::vector<unsigned>& GetDimensions() { return Dimensions; }
  unsigned CalcElemSize(unsigned dim) {
    unsigned result = 1;
    assert((dim == 0 || dim < Dimensions.size()) && "Out of bound");
    for (size_t i = dim + 1; i < Dimensions.size(); i++)
      result *= Dimensions[i];

    return result * (BitWidth / 8);
  }
  unsigned GetElemByteOffset(const unsigned StructElemIndex) const {
    assert(StructElemIndex < MembersTypeList.size() && "Out of bound access");

    unsigned ByteOffset = 0;
    for (size_t i = 0; i < StructElemIndex; i++)
      ByteOffset += MembersTypeList[i].GetByteSize();

    return ByteOffset;
  }
  size_t GetBitSize() const {

    return BitWidth;
  }

  size_t GetByteSize() const;

  IRType GetBaseType() const { return IRType(Kind, BitWidth); }

  void SetStructName(std::string &str) {StructName = str;}
  const std::string &GetStructName() const { return StructName; }

  std::vector<IRType> &GetMemberTypes() {return MembersTypeList;}

  std::string AsString() const;

private:
  uint8_t BitWidth;
  uint8_t PointerLevel = 0;
  std::string StructName;
  std::vector<IRType> MembersTypeList;
  std::vector<unsigned> Dimensions;
  TKind Kind;
};

#endif
