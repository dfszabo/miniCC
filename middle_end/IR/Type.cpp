#include "Type.hpp"

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
  case NONE:
    return "void";
  default:
    assert(!"Invalid type.");
    break;
  }

  Str += std::to_string(BitWidth);

  if (NumberOfElements > 1)
    Str = "[" + Str + " x " + std::to_string(NumberOfElements) + "]";

  return Str;
}
