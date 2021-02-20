#include "RISCVRegisterInfo.hpp"
#include <cassert>

using namespace RISCV;

RISCVRegisterInfo::RISCVRegisterInfo() {
  // FIXME: use the .def file somehow
  Registers[0] = TargetRegister::Create(ZERO, 32, "x0", "zero");
  Registers[1] = TargetRegister::Create(RA, 32, "x1", "ra");
  Registers[2] = TargetRegister::Create(SP, 32, "x2", "sp");
  Registers[3] = TargetRegister::Create(GP, 32, "x3", "gp");
  Registers[4] = TargetRegister::Create(TP, 32, "x4", "tp");
  Registers[5] = TargetRegister::Create(T0, 32, "x5", "t0");
  Registers[6] = TargetRegister::Create(T1, 32, "x6", "t1");
  Registers[7] = TargetRegister::Create(T2, 32, "x7", "t2");
  Registers[8] = TargetRegister::Create(S0, 32, "x8", "s0");
  Registers[9] = TargetRegister::Create(S1, 32, "x9", "s1");
  Registers[10] = TargetRegister::Create(A0, 32, "x10", "a0");
  Registers[11] = TargetRegister::Create(A1, 32, "x11", "a1");
  Registers[12] = TargetRegister::Create(A2, 32, "x12", "a2");
  Registers[13] = TargetRegister::Create(A3, 32, "x13", "a3");
  Registers[14] = TargetRegister::Create(A4, 32, "x14", "a4");
  Registers[15] = TargetRegister::Create(A5, 32, "x15", "a5");
  Registers[16] = TargetRegister::Create(A6, 32, "x16", "a6");
  Registers[17] = TargetRegister::Create(A7, 32, "x17", "a7");
  Registers[18] = TargetRegister::Create(S2, 32, "x18", "s2");
  Registers[19] = TargetRegister::Create(S3, 32, "x19", "s3");
  Registers[20] = TargetRegister::Create(S4, 32, "x20", "s4");
  Registers[21] = TargetRegister::Create(S5, 32, "x21", "s5");
  Registers[22] = TargetRegister::Create(S6, 32, "x22", "s6");
  Registers[23] = TargetRegister::Create(S7, 32, "x23", "s7");
  Registers[24] = TargetRegister::Create(S8, 32, "x24", "s8");
  Registers[25] = TargetRegister::Create(S9, 32, "x25", "s9");
  Registers[26] = TargetRegister::Create(S10, 32, "x26", "s10");
  Registers[27] = TargetRegister::Create(S11, 32, "x27", "s11");
  Registers[28] = TargetRegister::Create(T3, 32, "x28", "t3");
  Registers[29] = TargetRegister::Create(T4, 32, "x29", "t4");
  Registers[30] = TargetRegister::Create(T5, 32, "x30", "t5");
  Registers[31] = TargetRegister::Create(T6, 32, "x31", "t6");
}

TargetRegister *RISCVRegisterInfo::GetRegister(unsigned i) {
  assert(i < 32 && "Out of bound access");
  return &Registers[i];
}

TargetRegister *RISCVRegisterInfo::GetRegisterByID(unsigned i) {
  assert(i < 32 && "Out of bound access");
  return &Registers[i - 1];
}

unsigned RISCVRegisterInfo::GetFrameRegister() { return S0; }

unsigned RISCVRegisterInfo::GetStackRegister() { return SP; }
