#include "Type.hpp"

size_t IRType::GetByteSize() const {
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
  // TODO: Change the hard coded 32 to the target pointer size
  return (32 * NumberOfElements + 7) / 8;
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
    //Str = "[" + Str + " x " + std::to_string(NumberOfElements) + "]";
    for (auto CurrentDim : Dimensions)
      Str = "[" + Str + " x " + std::to_string(CurrentDim) + "]";
  }

  std::string PtrStr;
  for (auto i = 0; i < PointerLevel; i++)
    PtrStr += "*";

  return PtrStr + Str;
}
