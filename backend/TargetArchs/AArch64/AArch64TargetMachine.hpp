#include "../../MachineInstruction.hpp"
#include "../../TargetMachine.hpp"
#include "AArch64InstructionDefinitions.hpp"
#include "AArch64InstructionLegalizer.hpp"
#include "AArch64RegisterInfo.hpp"
#include "AArch64TargetABI.hpp"
#include <cassert>

namespace AArch64 {

class AArch64TargetMachine : public TargetMachine {
public:
  AArch64TargetMachine() {
    RegInfo = std::make_unique<AArch64RegisterInfo>();
    ABI = std::make_unique<AArch64TargetABI>(RegInfo.get());
    InstrDefs = std::make_unique<AArch64InstructionDefinitions>();
    Legalizer = std::make_unique<AArch64InstructionLegalizer>(this);
  }

  ~AArch64TargetMachine() override {}

  uint8_t GetPointerSize() override { return 64; }
  uint8_t GetIntSize() override { return 32; }
  uint8_t GetLongSize() override { return 64; }

  bool SelectAND(MachineInstruction *MI) override;
  bool SelectOR(MachineInstruction *MI) override;
  bool SelectXOR(MachineInstruction *MI) override;
  bool SelectLSL(MachineInstruction *MI) override;
  bool SelectLSR(MachineInstruction *MI) override;
  bool SelectADD(MachineInstruction *MI) override;
  bool SelectSUB(MachineInstruction *MI) override;
  bool SelectMUL(MachineInstruction *MI) override;
  bool SelectDIV(MachineInstruction *MI) override;
  bool SelectDIVU(MachineInstruction *MI) override;
  bool SelectMOD(MachineInstruction *MI) override;
  bool SelectMODU(MachineInstruction *MI) override;
  bool SelectCMP(MachineInstruction *MI) override;
  bool SelectCMPF(MachineInstruction *MI) override;
  bool SelectADDF(MachineInstruction *MI) override;
  bool SelectSUBF(MachineInstruction *MI) override;
  bool SelectMULF(MachineInstruction *MI) override;
  bool SelectDIVF(MachineInstruction *MI) override;
  bool SelectITOF(MachineInstruction *MI) override;
  bool SelectFTOI(MachineInstruction *MI) override;
  bool SelectSEXT(MachineInstruction *MI) override;
  bool SelectZEXT(MachineInstruction *MI) override;
  bool SelectTRUNC(MachineInstruction *MI) override;
  bool SelectZEXT_LOAD(MachineInstruction *MI) override;
  bool SelectLOAD_IMM(MachineInstruction *MI) override;
  bool SelectMOV(MachineInstruction *MI) override;
  bool SelectMOVF(MachineInstruction *MI) override;
  bool SelectLOAD(MachineInstruction *MI) override;
  bool SelectSTORE(MachineInstruction *MI) override;
  bool SelectSTACK_ADDRESS(MachineInstruction *MI) override;
  bool SelectBRANCH(MachineInstruction *MI) override;
  bool SelectJUMP(MachineInstruction *MI) override;
  bool SelectCALL(MachineInstruction *MI) override;
  bool SelectRET(MachineInstruction *MI) override;

  MachineInstruction *MaterializeConstant(MachineInstruction *MI,
                                          const uint64_t Constant,
                                          MachineOperand &Reg,
                                          const bool UseVRegAndMI = false);
  bool SelectThreeAddressInstruction(MachineInstruction *MI, const Opcodes rrr,
                                     const Opcodes rri, unsigned ImmSize = 12);
};

} // namespace AArch64