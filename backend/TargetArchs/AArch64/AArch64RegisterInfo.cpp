#include "AArch64RegisterInfo.hpp"
#include <cassert>

using namespace AArch64;

AArch64RegisterInfo::AArch64RegisterInfo() {
  // FIXME: use the .def file somehow
  Registers[0] = TargetRegister::Create(W0, 32, "w0", "");
  Registers[1] = TargetRegister::Create(W1, 32, "w1", "");
  Registers[2] = TargetRegister::Create(W2, 32, "w2", "");
  Registers[3] = TargetRegister::Create(W3, 32, "w3", "");
  Registers[4] = TargetRegister::Create(W4, 32, "w4", "");
  Registers[5] = TargetRegister::Create(W5, 32, "w5", "");
  Registers[6] = TargetRegister::Create(W6, 32, "w6", "");
  Registers[7] = TargetRegister::Create(W7, 32, "w7", "");
  Registers[8] = TargetRegister::Create(W8, 32, "w8", "");
  Registers[9] = TargetRegister::Create(W9, 32, "w9", "");
  Registers[10] = TargetRegister::Create(W10, 32, "w10", "");
  Registers[11] = TargetRegister::Create(W11, 32, "w11", "");
  Registers[12] = TargetRegister::Create(W12, 32, "w12", "");
  Registers[13] = TargetRegister::Create(W13, 32, "w13", "");
  Registers[14] = TargetRegister::Create(W14, 32, "w14", "");
  Registers[15] = TargetRegister::Create(W15, 32, "w15", "");
  Registers[16] = TargetRegister::Create(W16, 32, "w16", "");
  Registers[17] = TargetRegister::Create(W17, 32, "w17", "");
  Registers[18] = TargetRegister::Create(W18, 32, "w18", "");
  Registers[19] = TargetRegister::Create(W19, 32, "w19", "");
  Registers[20] = TargetRegister::Create(W20, 32, "w20", "");
  Registers[21] = TargetRegister::Create(W21, 32, "w21", "");
  Registers[22] = TargetRegister::Create(W22, 32, "w22", "");
  Registers[23] = TargetRegister::Create(W23, 32, "w23", "");
  Registers[24] = TargetRegister::Create(W24, 32, "w24", "");
  Registers[25] = TargetRegister::Create(W25, 32, "w25", "");
  Registers[26] = TargetRegister::Create(W26, 32, "w26", "");
  Registers[27] = TargetRegister::Create(W27, 32, "w27", "");
  Registers[28] = TargetRegister::Create(W28, 32, "w28", "");
  Registers[29] = TargetRegister::Create(W29, 32, "w29", "");
  Registers[30] = TargetRegister::Create(W30, 32, "w30", "");
  Registers[31] = TargetRegister::Create(W31, 32, "w31", "sp");
  Registers[32] = TargetRegister::Create(SP, 32, "sp", "");
  Registers[33] = TargetRegister::Create(XZR, 32, "wzr", "");
  Registers[34] = TargetRegister::Create(PC, 32, "pc", "");
}

TargetRegister *AArch64RegisterInfo::GetRegister(unsigned i) {
  assert(i < 35 && "Out of bound access");
  return &Registers[i];
}

TargetRegister *AArch64RegisterInfo::GetRegisterByID(unsigned i) {
  assert(i < 35 && "Out of bound access");
  return &Registers[i - 1];
}

unsigned AArch64RegisterInfo::GetFrameRegister() { return 29; }

unsigned AArch64RegisterInfo::GetStackRegister() { return 32; }
