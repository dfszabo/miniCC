#include "RISCVTargetABI.hpp"
#include "RISCVRegisterInfo.hpp"

using namespace RISCV;

RISCVTargetABI::RISCVTargetABI(RegisterInfo *RI) {
  StackAlignment = 16;
  MaxStructSize = 64;

  // aX registers
  for (unsigned i = A0; i <= A7; i++)
    ArgumentRegisters.push_back(RI->GetRegisterByID(i));

  // s0
  CalleeSavedRegisters.push_back(RI->GetRegisterByID(S0));
  // s1
  CalleeSavedRegisters.push_back(RI->GetRegisterByID(S1));
  // s2-s11
  for (unsigned i = S2; i <= S11; i++)
    CalleeSavedRegisters.push_back(RI->GetRegisterByID(i));

  // a0-a7
  for (unsigned i = A0; i <= A7; i++)
    CallerSavedRegisters.push_back(RI->GetRegisterByID(i));
  // t0-t2
  for (unsigned i = T0; i <= T2; i++)
    CallerSavedRegisters.push_back(RI->GetRegisterByID(i));
  // t3-t6
  for (unsigned i = T3; i <= T6; i++)
    CallerSavedRegisters.push_back(RI->GetRegisterByID(i));

  // a0
  ReturnRegisters.push_back(RI->GetRegisterByID(A0));
  // a1
  ReturnRegisters.push_back(RI->GetRegisterByID(A1));
}
