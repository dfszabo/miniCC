#include "IRType.hpp"
#include "../../backend/Support.hpp"
#include "../../backend/TargetMachine.hpp"

void IRType::ReduceDimension() {
  if (Dimensions.size() > 0)
    Dimensions.erase(Dimensions.begin());
}

unsigned IRType::GetStructMaxAlignment(TargetMachine *TM) const {
  unsigned alignment = 1;

  for (auto &type : MembersTypeList)
    if (type.IsPTR()) {
      unsigned ptrSize = TM ? TM->GetPointerSize() / 8 : 8;
      alignment = std::max(alignment, ptrSize);
    } else if (type.IsArray()) {
      if (type.GetBaseType().IsScalar())
        alignment =
            std::max(alignment, (unsigned)type.GetBaseType().GetByteSize(TM));
#if 0
      else if (type.GetBaseType().IsScalar())
        // TODO: this will not work currently, since GetBaseType implementation
        // does not cover structs
        alignment = type.GetBaseType().GetStructMaxAlignment(TM);
#endif
      else
        assert(!"Unhandled array base type");
    } else if (type.IsScalar())
      alignment = std::max(alignment, (unsigned)type.GetByteSize(TM));
    else if (type.IsStruct())
      alignment = std::max(alignment, (unsigned)type.GetStructMaxAlignment(TM));
    else
      assert(!"Unhandled type");

  return alignment;
}

unsigned IRType::GetElemByteOffset(const unsigned StructElemIndex,
                                   TargetMachine *TM) const {
  assert(StructElemIndex < MembersTypeList.size() && "Out of bound access");

  unsigned ByteOffset = 0;
  for (size_t i = 0; i < StructElemIndex; i++) {
    const unsigned Size = MembersTypeList[i].GetByteSize();
    ByteOffset = GetNextAlignedValue(ByteOffset, Size);
    ByteOffset += Size;
  }

  const unsigned MaxAlignment = GetStructMaxAlignment(TM);
  ByteOffset = GetNextAlignedValue(ByteOffset, MaxAlignment);
  return ByteOffset;
}

size_t IRType::GetByteSize(TargetMachine *TM) const {
  unsigned NumberOfElements = 1;

  if (Dimensions.size() > 0)
    for (size_t i = 0; i < Dimensions.size(); i++)
      NumberOfElements *= Dimensions[i];

  if (IsStruct() && !IsPTR()) {
    unsigned Result = 0;
    const unsigned Alignment = GetStructMaxAlignment(TM);

    for (auto &type : MembersTypeList) {
      auto size = type.GetByteSize();
      Result = GetNextAlignedValue(Result, size);
      Result += size;
    }

    Result = GetNextAlignedValue(Result, Alignment);
    return Result;
  }

  if (PointerLevel == 0)
    return (BitWidth * NumberOfElements + 7) / 8;

  // in case if it is a pointer type, then ask the target for the pointer size
  // or if it was not given then the default size is 64
  unsigned PtrSize = 64;
  if (TM)
    PtrSize = TM->GetPointerSize();

  return (PtrSize * NumberOfElements + 7) / 8;
}

size_t IRType::GetBaseTypeByteSize(TargetMachine *TM) const {
  auto TypeCopy = *this;
  TypeCopy.SetPointerLevel(0);
  return TypeCopy.GetByteSize(TM);
}

std::string IRType::AsString() const {
  std::string Str;

  switch (Kind) {
  case FP:
    Str += "f";
    break;
  case UINT:
    Str += "u";
    break;
  case SINT:
    Str += "i";
    break;
  case STRUCT:
    Str += "struct." + StructName;
    break;
  case NONE:
    return "void";
  default:
    assert(!"Invalid type.");
    break;
  }

  if (Kind != STRUCT)
    Str += std::to_string(BitWidth);

  if (Dimensions.size() > 0) {
    for (int i = Dimensions.size() - 1; i >= 0; i--)
      Str = "[" + std::to_string(Dimensions[i])  + " x " + Str + "]";
  }

  std::string PtrStr;
  for (auto i = 0; i < PointerLevel; i++)
    PtrStr += "*";

  return PtrStr + Str;
}
