#include "Type.hpp"

std::string Type::ToString(const Type *t) {
  std::string Result = "";

  switch (t->GetTypeVariant()) {
  case Double:
    Result = "double";
    break;
  case Char:
    Result = "char";
    break;
  case UnsignedChar:
    Result = "unsigned char";
    break;
  case Short:
    Result = "short";
    break;
  case UnsignedShort:
    Result = "unsigned short";
    break;
  case Int:
    Result = "int";
    break;
  case UnsignedInt:
    Result = "unsigned int";
    break;
  case Long:
    Result = "long";
    break;
  case UnsignedLong:
    Result = "unsigned long";
    break;
  case LongLong:
    Result = "long long";
    break;
  case UnsignedLongLong:
    Result = "unsigned long long";
    break;
  case Void:
    Result = "void";
    break;
  case Composite:
    Result = t->GetName();
    break;
  case Invalid:
    return "invalid";
  default:
    assert(!"Unknown type.");
    break;
  }

  for (size_t i = 0; i < t->GetPointerLevel(); i++)
    Result.push_back('*');

  return Result;
}

std::string Type::ToString() const {
  if (IsFunction()) {
    auto TyStr = Type::ToString(this);
    auto ArgSize = ParameterList.size();
    if (ArgSize > 0)
      TyStr += " (";
    for (size_t i = 0; i < ArgSize; i++) {
      TyStr += Type::ToString(&ParameterList[i]);
      if (i + 1 < ArgSize)
        TyStr += ",";
      else
        TyStr += ")";
    }
    return TyStr;
  } else if (Kind == Array) {
    auto TyStr = Type::ToString(this);

    for (size_t i = 0; i < Dimensions.size(); i++)
      TyStr += "[" + std::to_string(Dimensions[i]) + "]";
    return TyStr;
  } else {
    return Type::ToString(this);
  }
}
