#include "AArch64RegisterInfo.hpp"
#include "AArch64InstructionDefinitions.hpp"
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

  Registers[64] = TargetRegister::Create(D0, 64, "d0", "", {}, true);
  Registers[65] = TargetRegister::Create(D1, 64, "d1", "", {}, true);
  Registers[66] = TargetRegister::Create(D2, 64, "d2", "", {}, true);
  Registers[67] = TargetRegister::Create(D3, 64, "d3", "", {}, true);
  Registers[68] = TargetRegister::Create(D4, 64, "d4", "", {}, true);
  Registers[69] = TargetRegister::Create(D5, 64, "d5", "", {}, true);
  Registers[70] = TargetRegister::Create(D6, 64, "d6", "", {}, true);
  Registers[71] = TargetRegister::Create(D7, 64, "d7", "", {}, true);
  Registers[72] = TargetRegister::Create(D8, 64, "d8", "", {}, true);
  Registers[73] = TargetRegister::Create(D9, 64, "d9", "", {}, true);
  Registers[74] = TargetRegister::Create(D10, 64, "d10", "", {}, true);
  Registers[75] = TargetRegister::Create(D11, 64, "d11", "", {}, true);
  Registers[76] = TargetRegister::Create(D12, 64, "d12", "", {}, true);
  Registers[77] = TargetRegister::Create(D13, 64, "d13", "", {}, true);
  Registers[78] = TargetRegister::Create(D14, 64, "d14", "", {}, true);
  Registers[79] = TargetRegister::Create(D15, 64, "d15", "", {}, true);
  Registers[80] = TargetRegister::Create(D16, 64, "d16", "", {}, true);
  Registers[81] = TargetRegister::Create(D17, 64, "d17", "", {}, true);
  Registers[82] = TargetRegister::Create(D18, 64, "d18", "", {}, true);
  Registers[83] = TargetRegister::Create(D19, 64, "d19", "", {}, true);
  Registers[84] = TargetRegister::Create(D20, 64, "d20", "", {}, true);
  Registers[85] = TargetRegister::Create(D21, 64, "d21", "", {}, true);
  Registers[86] = TargetRegister::Create(D22, 64, "d22", "", {}, true);
  Registers[87] = TargetRegister::Create(D23, 64, "d23", "", {}, true);
  Registers[88] = TargetRegister::Create(D24, 64, "d24", "", {}, true);
  Registers[89] = TargetRegister::Create(D25, 64, "d25", "", {}, true);
  Registers[90] = TargetRegister::Create(D26, 64, "d26", "", {}, true);
  Registers[91] = TargetRegister::Create(D27, 64, "d27", "", {}, true);
  Registers[92] = TargetRegister::Create(D28, 64, "d28", "", {}, true);
  Registers[93] = TargetRegister::Create(D29, 64, "d29", "", {}, true);
  Registers[94] = TargetRegister::Create(D30, 64, "d30", "", {}, true);
  Registers[95] = TargetRegister::Create(D31, 64, "d31", "", {}, true);

  Registers[96] = TargetRegister::Create(SP, 64, "sp", "");
  Registers[97] = TargetRegister::Create(WZR, 32, "wzr", "");
  Registers[98] = TargetRegister::Create(XZR, 64, "xzr", "");
  Registers[99] = TargetRegister::Create(PC, 64, "pc", "");

  RegClassEnumStrings = {"gpr", "gpr32", "gpr64", "fpr", "fpr32", "fpr64"};
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
  assert(i < REGISTERS_END - 1 && "Out of bound access");
  return &Registers[i];
}

TargetRegister *AArch64RegisterInfo::GetRegisterByID(unsigned i) {
  assert(i != 0 && i < REGISTERS_END && "Out of bound access");
  return &Registers[i - 1];
}

unsigned AArch64RegisterInfo::GetFrameRegister() { return 61; }

unsigned AArch64RegisterInfo::GetLinkRegister() { return X30; }

unsigned AArch64RegisterInfo::GetStackRegister() { return X31; }

unsigned AArch64RegisterInfo::GetStructPtrRegister() { return X8; }

unsigned AArch64RegisterInfo::GetZeroRegister(const unsigned BitWidth) {
  if (BitWidth <= 32)
    return WZR; 
  return XZR;
}

unsigned AArch64RegisterInfo::GetRegisterClass(const unsigned BitWidth,
                                               const bool IsFP) {
  if (IsFP) {
    if (BitWidth <= 32)
      return FPR32;
    else
      return FPR64;
  } else {
    if (BitWidth <= 32)
      return GPR32;
    else
      return GPR64;
  }
}

std::string AArch64RegisterInfo::GetRegClassString(const unsigned RegClass) {
  assert(RegClass < RegClassEnumStrings.size());
  return RegClassEnumStrings[RegClass];
}

unsigned AArch64RegisterInfo::GetRegClassFromReg(const unsigned Reg) {
  if (Reg >= W0 && Reg <= W31)
    return GPR32;
  else if (Reg >= X0 && Reg <= X31)
    return GPR64;
  else if (Reg >= D0 && Reg <= D31)
    return FPR64;
  else if (Reg == XZR || Reg == SP)
    return GPR64;
  else if (Reg == WZR)
    return GPR32;

  assert(!"Unknown register");
  return ~0;
}

unsigned AArch64RegisterInfo::GetRegClassRegsSize(const unsigned RegClass) {
  switch (RegClass) {
  case GPR32:
  case FPR32:
    return 32;
  case GPR:
  case GPR64:
  case FPR:
  case FPR64:
    return 64;
  default:
    assert(!"Unknown register class");
  }

  return ~0;
}
