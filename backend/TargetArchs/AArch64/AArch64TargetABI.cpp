#include "AArch64TargetABI.hpp"
#include "AArch64RegisterInfo.hpp"

using namespace AArch64;

AArch64TargetABI::AArch64TargetABI(RegisterInfo *RI) {
  StackAlignment = 16;
  MaxStructSize = 128;
  CLib = true;

  /// Argument registers

  // x0-x7
  for (unsigned i = X0; i <= X7; i++)
    ArgumentRegisters.push_back(RI->GetRegisterByID(i));

  FirstFPArgRegIdx = ArgumentRegisters.size();

  // d0-d7
  for (unsigned i = D0; i <= D7; i++)
    ArgumentRegisters.push_back(RI->GetRegisterByID(i));


  /// Callee/Caller saved registers

  // TODO: make maybe a new vector for callee saved GPR regs
  // so excluding special ones like sp, fp and lr

  // x19-x28
  for (unsigned i = X19; i <= X28; i++)
    CalleeSavedRegisters.push_back(RI->GetRegisterByID(i));

  // x0-x7
  for (unsigned i = X0; i <= X7; i++)
    CallerSavedRegisters.push_back(RI->GetRegisterByID(i));
  // x9-x15
  for (unsigned i = X9; i <= X15; i++)
    CallerSavedRegisters.push_back(RI->GetRegisterByID(i));

  // d0-d7
  for (unsigned i = D0; i <= D7; i++)
    CallerSavedRegisters.push_back(RI->GetRegisterByID(i));
  // d9-d15
  for (unsigned i = D9; i <= D15; i++)
    CallerSavedRegisters.push_back(RI->GetRegisterByID(i));


  /// Return registers

  // x0-x7
  for (unsigned i = X0; i <= X7; i++)
    ReturnRegisters.push_back(RI->GetRegisterByID(i));

  FirstFPRetRegIdx = ReturnRegisters.size();

  // d0-d7
  for (unsigned i = D0; i <= D7; i++)
    ReturnRegisters.push_back(RI->GetRegisterByID(i));
}
