#ifndef LOW_LEVEL_TYPE_HPP
#define LOW_LEVEL_TYPE_HPP

#include <string>
#include <cassert>

class LowLevelType {
public:
  enum Type : unsigned {
    INVALID,
    POINTER,
    SCALAR,
  };

  LowLevelType() : Type(INVALID) {}
  LowLevelType(unsigned Type) : Type(Type) {}

  void SetBitWidth(unsigned BW) { BitWidth = BW; }
  unsigned GetBitWidth() const { return BitWidth; }

  static LowLevelType CreateScalar(unsigned BitWidth) {
    LowLevelType LLT(SCALAR);
    LLT.SetBitWidth(BitWidth);
    return LLT;
  }

  static LowLevelType CreatePTR(unsigned BitWidth = 32) {
    LowLevelType LLT(POINTER);
    LLT.SetBitWidth(BitWidth);
    return LLT;
  }

  bool IsValid() const { return Type != INVALID; }
  bool IsScalar() const { return Type == SCALAR; }
  bool IsPointer() const { return Type == POINTER; }

  std::string ToString() const {
    std::string str;

    if (Type == SCALAR)
      str = "s";
    else if (Type == POINTER)
      str = "p";
    else
      assert(!"Invalid type");

    str += std::to_string(BitWidth);

    return str;
  }

private:
  unsigned Type = INVALID;
  unsigned BitWidth;
};

#endif
