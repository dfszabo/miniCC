#ifndef LOW_LEVEL_TYPE_HPP
#define LOW_LEVEL_TYPE_HPP

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
  unsigned GetBitWidth() { return BitWidth; }

  static LowLevelType CreateINT(unsigned BitWidth) {
    LowLevelType LLT(INTEGER);
    LLT.SetBitWidth(BitWidth);
    return LLT;
  }

  bool IsInteger() const { return Type == INTEGER; }

private:
  unsigned Type = INVALID;
  unsigned BitWidth;
};

#endif
