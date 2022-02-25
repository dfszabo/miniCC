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

  RegList &GetCalleeSavedRegisters() { return CalleeSavedRegisters; }

  RegList &GetReturnRegisters() { return ReturnRegisters; }
  void SetReturnRegisters(RegList ReturnRegs) { ReturnRegisters = ReturnRegs; }

  size_t GetFirstFPArgRegIdx() const { return FirstFPArgRegIdx; }
  size_t GetFirstFPRetRegIdx() const { return FirstFPRetRegIdx; }

protected:
  unsigned StackAlignment = ~0;
  unsigned MaxStructSize = ~0;
  unsigned FirstFPArgRegIdx = 0;
  unsigned FirstFPRetRegIdx = 0;

  /// Targets should fill this by the general registers they use for passing
  /// integer values. Then fill it with the floating point registers.
  RegList ArgumentRegisters;
  RegList CalleeSavedRegisters;
  RegList CallerSavedRegisters;
  RegList ReturnRegisters;
};

#endif
