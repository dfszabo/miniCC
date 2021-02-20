#include "AArch64TargetABI.hpp"

using namespace AArch64;

AArch64TargetABI::AArch64TargetABI(RegisterInfo *RI) {
  StackAlignment = 16;

  // w0-w7
  for (int i = 0; i <= 7; i++)
    ArgumentRegisters.push_back(RI->GetRegister(i));

  // sp
  CalleSavedRegisters.push_back(RI->GetRegister(32));
  // fp
  CalleSavedRegisters.push_back(RI->GetRegister(29));
  // w19-w28
  for (int i = 19; i <= 28; i++)
    CalleSavedRegisters.push_back(RI->GetRegister(i));

  // w0-w7
  for (int i = 0; i <= 7; i++)
    CallerSavedRegisters.push_back(RI->GetRegister(i));
  // w9-w15
  for (int i = 9; i <= 15; i++)
    CallerSavedRegisters.push_back(RI->GetRegister(i));

  // w0-w7
  for (int i = 0; i <= 7; i++)
    ReturnRegisters.push_back(RI->GetRegister(i));
}
