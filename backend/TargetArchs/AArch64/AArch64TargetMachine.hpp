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

  bool SelectADD(MachineInstruction *MI) override;
  bool SelectSUB(MachineInstruction *MI) override;
  bool SelectMUL(MachineInstruction *MI) override;
  bool SelectDIV(MachineInstruction *MI) override;
  bool SelectMOD(MachineInstruction *MI) override;
  bool SelectCMP(MachineInstruction *MI) override;
  bool SelectSEXT(MachineInstruction *MI) override;
  bool SelectTRUNC(MachineInstruction *MI) override;
  bool SelectLOAD_IMM(MachineInstruction *MI) override;
  bool SelectMOV(MachineInstruction *MI) override;
  bool SelectLOAD(MachineInstruction *MI) override;
  bool SelectSTORE(MachineInstruction *MI) override;
  bool SelectSTACK_ADDRESS(MachineInstruction *MI) override;
  bool SelectBRANCH(MachineInstruction *MI) override;
  bool SelectJUMP(MachineInstruction *MI) override;
  bool SelectCALL(MachineInstruction *MI) override;
  bool SelectRET(MachineInstruction *MI) override;
};

} // namespace AArch64