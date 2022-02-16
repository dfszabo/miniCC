#include "IRType.hpp"
#include "../../backend/TargetMachine.hpp"

void IRType::ReduceDimension() {
  if (Dimensions.size() > 0)
    Dimensions.erase(Dimensions.begin());
}

size_t IRType::GetByteSize(TargetMachine *TM) const {
  unsigned NumberOfElements = 1;

  if (Dimensions.size() > 0)
    for (size_t i = 0; i < Dimensions.size(); i++)
      NumberOfElements *= Dimensions[i];

  if (IsStruct()) {
    unsigned Result = 0;
    for (auto &type : MembersTypeList)
      Result += type.GetByteSize();
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
    Str += StructName;
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
