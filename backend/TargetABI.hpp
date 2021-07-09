#ifndef TARGETABI_HPP
#define TARGETABI_HPP

#include "TargetRegister.hpp"
#include <vector>

class TargetABI {
  using RegList = std::vector<TargetRegister *>;

public:
  TargetABI() {}

  unsigned GetStackAlignment() const { return StackAlignment; }
  void SetStackAlignment(unsigned Alignment) { StackAlignment = Alignment; }

  unsigned GetMaxStructSizePassedByValue() const { return MaxStructSize; }
  void SetMaxStructSizePassedByValue(unsigned S) { MaxStructSize = S; }

  RegList &GetArgumentRegisters() { return ArgumentRegisters; }
  void SetArgumentRegisters(RegList ArgRegs) { ArgumentRegisters = ArgRegs; }

  RegList &GetCallerSavedRegisters() { return CallerSavedRegisters; }
  void SetCallerSavedRegisters(RegList CallerSaved) {
    CallerSavedRegisters = CallerSaved;
  }

  RegList &GetReturnRegisters() { return ReturnRegisters; }
  void SetReturnRegisters(RegList ReturnRegs) { ReturnRegisters = ReturnRegs; }

protected:
  unsigned StackAlignment = ~0;
  unsigned MaxStructSize = ~0;
  RegList ArgumentRegisters;
  RegList CalleSavedRegisters;
  RegList CallerSavedRegisters;
  RegList ReturnRegisters;
};

#endif
