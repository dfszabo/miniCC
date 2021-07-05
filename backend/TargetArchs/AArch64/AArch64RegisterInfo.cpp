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

  Registers[32] = TargetRegister::Create(X0, 64, "x0", "", {W0});
  Registers[33] = TargetRegister::Create(X1, 64, "x1", "", {W1});
  Registers[34] = TargetRegister::Create(X2, 64, "x2", "", {W2});
  Registers[35] = TargetRegister::Create(X3, 64, "x3", "", {W3});
  Registers[36] = TargetRegister::Create(X4, 64, "x4", "", {W4});
  Registers[37] = TargetRegister::Create(X5, 64, "x5", "", {W5});
  Registers[38] = TargetRegister::Create(X6, 64, "x6", "", {W6});
  Registers[39] = TargetRegister::Create(X7, 64, "x7", "", {W7});
  Registers[40] = TargetRegister::Create(X8, 64, "x8", "", {W8});
  Registers[41] = TargetRegister::Create(X9, 64, "x9", "", {W9});
  Registers[42] = TargetRegister::Create(X10, 64, "x10", "", {W10});
  Registers[43] = TargetRegister::Create(X11, 64, "x11", "", {W11});
  Registers[44] = TargetRegister::Create(X12, 64, "x12", "", {W12});
  Registers[45] = TargetRegister::Create(X13, 64, "x13", "", {W13});
  Registers[46] = TargetRegister::Create(X14, 64, "x14", "", {W14});
  Registers[47] = TargetRegister::Create(X15, 64, "x15", "", {W15});
  Registers[48] = TargetRegister::Create(X16, 64, "x16", "", {W16});
  Registers[49] = TargetRegister::Create(X17, 64, "x17", "", {W17});
  Registers[50] = TargetRegister::Create(X18, 64, "x18", "", {W18});
  Registers[51] = TargetRegister::Create(X19, 64, "x19", "", {W19});
  Registers[52] = TargetRegister::Create(X20, 64, "x20", "", {W20});
  Registers[53] = TargetRegister::Create(X21, 64, "x21", "", {W21});
  Registers[54] = TargetRegister::Create(X22, 64, "x22", "", {W22});
  Registers[55] = TargetRegister::Create(X23, 64, "x23", "", {W23});
  Registers[56] = TargetRegister::Create(X24, 64, "x24", "", {W24});
  Registers[57] = TargetRegister::Create(X25, 64, "x25", "", {W25});
  Registers[58] = TargetRegister::Create(X26, 64, "x26", "", {W26});
  Registers[59] = TargetRegister::Create(X27, 64, "x27", "", {W27});
  Registers[60] = TargetRegister::Create(X28, 64, "x28", "", {W28});
  Registers[61] = TargetRegister::Create(X29, 64, "x29", "", {W29});
  Registers[62] = TargetRegister::Create(X30, 64, "x30", "", {W30});
  Registers[63] = TargetRegister::Create(X31, 64, "x31", "sp", {W31});

  Registers[64] = TargetRegister::Create(SP, 64, "sp", "");
  Registers[65] = TargetRegister::Create(XZR, 64, "wzr", "");
  Registers[66] = TargetRegister::Create(PC, 64, "pc", "");
}

TargetRegister *AArch64RegisterInfo::GetParentReg(unsigned ID) {
  for (size_t i = 0; i < sizeof Registers / sizeof Registers[0]; i++)
    if (!Registers[i].GetSubRegs().empty())
      for (auto SubReg : Registers[i].GetSubRegs())
        if (SubReg == ID)
          return &Registers[i];

  return nullptr;
}

TargetRegister *AArch64RegisterInfo::GetRegister(unsigned i) {
  assert(i < 67 && "Out of bound access");
  return &Registers[i];
}

TargetRegister *AArch64RegisterInfo::GetRegisterByID(unsigned i) {
  assert(i != 0 && i < 67 && "Out of bound access");
  return &Registers[i - 1];
}

unsigned AArch64RegisterInfo::GetFrameRegister() { return 61; }

unsigned AArch64RegisterInfo::GetLinkRegister() { return X30; }

unsigned AArch64RegisterInfo::GetStackRegister() { return 64; }

unsigned AArch64RegisterInfo::GetZeroRegister() { return XZR; }
