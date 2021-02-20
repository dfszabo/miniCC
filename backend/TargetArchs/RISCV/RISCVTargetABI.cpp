#include "RISCVTargetABI.hpp"

using namespace RISCV;

RISCVTargetABI::RISCVTargetABI(RegisterInfo *RI) {
  StackAlignment = 16;

  // aX registers
  for (int i = 10; i <= 17; i++)
    ArgumentRegisters.push_back(RI->GetRegister(i));

  // sp
  CalleSavedRegisters.push_back(RI->GetRegister(2));
  // s0
  CalleSavedRegisters.push_back(RI->GetRegister(8));
  // s1
  CalleSavedRegisters.push_back(RI->GetRegister(9));
  // s2-s11
  for (int i = 18; i <= 27; i++)
    CalleSavedRegisters.push_back(RI->GetRegister(i));

  // a0-a7
  for (int i = 10; i <= 17; i++)
    CallerSavedRegisters.push_back(RI->GetRegister(i));
  // t0-t2
  for (int i = 5; i <= 7; i++)
    CallerSavedRegisters.push_back(RI->GetRegister(i));
  // t3-t6
  for (int i = 28; i <= 31; i++)
    CallerSavedRegisters.push_back(RI->GetRegister(i));
  // ra
  CallerSavedRegisters.push_back(RI->GetRegister(1));

  // a0
  ReturnRegisters.push_back(RI->GetRegister(10));
  // a1
  ReturnRegisters.push_back(RI->GetRegister(11));
}
