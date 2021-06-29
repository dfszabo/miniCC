#ifndef LOW_LEVEL_TYPE_HPP
#define LOW_LEVEL_TYPE_HPP

#include <string>

class LowLevelType {
public:
  enum Type : unsigned {
    INVALID,
    POINTER,
    INTEGER,
    FLOATING_POINT,
  };

  LowLevelType() : Type(INVALID) {}
  LowLevelType(unsigned Type) : Type(Type) {}

  void SetBitWidth(unsigned BW) { BitWidth = BW; }
  unsigned GetBitWidth() const { return BitWidth; }

  static LowLevelType CreateINT(unsigned BitWidth) {
    LowLevelType LLT(INTEGER);
    LLT.SetBitWidth(BitWidth);
    return LLT;
  }

  static LowLevelType CreatePTR(unsigned BitWidth = 32) {
    LowLevelType LLT(POINTER);
    LLT.SetBitWidth(BitWidth);
    return LLT;
  }

  bool IsValid() const { return Type != INVALID; }
  bool IsInteger() const { return Type == INTEGER; }
  bool IsPointer() const { return Type == POINTER; }

  std::string ToString() const {
    std::string str;

    if (Type == INTEGER)
      str = "i";
    else if (Type == FLOATING_POINT)
      str = "f";
    else if (Type == POINTER)
      str = "p";

    str += std::to_string(BitWidth);

    return str;
  }

private:
  unsigned Type = INVALID;
  unsigned BitWidth;
};

#endif
