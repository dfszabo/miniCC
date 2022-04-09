#include "../../MachineInstruction.hpp"
#include "../../TargetMachine.hpp"
#include "RISCVInstructionDefinitions.hpp"
#include "RISCVInstructionLegalizer.hpp"
#include "RISCVRegisterInfo.hpp"
#include "RISCVTargetABI.hpp"
#include <cassert>

namespace RISCV {

class RISCVTargetMachine : public TargetMachine {
public:
  RISCVTargetMachine() {
    RegInfo = std::make_unique<RISCVRegisterInfo>();
    ABI = std::make_unique<RISCVTargetABI>(RegInfo.get());
    InstrDefs = std::make_unique<RISCVInstructionDefinitions>();
    Legalizer = std::make_unique<RISCVInstructionLegalizer>(this);
  }

  ~RISCVTargetMachine() override {}

  uint8_t GetPointerSize() override { return 32; }
  uint8_t GetIntSize() override { return 32; }
  uint8_t GetLongSize() override { return 32; }

  bool SelectThreeAddressInstruction(MachineInstruction *MI, const Opcodes rrr,
                                     const Opcodes rri, unsigned ImmSize = 12);

  bool SelectThreeAddressInstruction(MachineInstruction *MI, const Opcodes rrr);

  bool SelectAND(MachineInstruction *MI) override;
  bool SelectOR(MachineInstruction *MI) override;
  bool SelectXOR(MachineInstruction *MI) override;
  bool SelectLSL(MachineInstruction *MI) override;
  bool SelectLSR(MachineInstruction *MI) override;
  bool SelectADD(MachineInstruction *MI) override;
  bool SelectSUB(MachineInstruction *MI) override;
  bool SelectMUL(MachineInstruction *MI) override;
  bool SelectMULHU(MachineInstruction *MI) override;
  bool SelectDIV(MachineInstruction *MI) override;
  bool SelectDIVU(MachineInstruction *MI) override;
  bool SelectMOD(MachineInstruction *MI) override;
  bool SelectMODU(MachineInstruction *MI) override;
  bool SelectCMP(MachineInstruction *MI) override;

  bool SelectSEXT(MachineInstruction *MI) override;
  bool SelectZEXT(MachineInstruction *MI) override;
  bool SelectTRUNC(MachineInstruction *MI) override;
  bool SelectZEXT_LOAD(MachineInstruction *MI) override;
  bool SelectLOAD(MachineInstruction *MI) override;
  bool SelectLOAD_IMM(MachineInstruction *MI) override;
  bool SelectMOV(MachineInstruction *MI) override;
  bool SelectSTORE(MachineInstruction *MI) override;
  bool SelectSTACK_ADDRESS(MachineInstruction *MI) override;
  bool SelectGLOBAL_ADDRESS(MachineInstruction *MI) override;
  bool SelectBRANCH(MachineInstruction *MI) override;
  bool SelectJUMP(MachineInstruction *MI) override;
  bool SelectCALL(MachineInstruction *MI) override;
  bool SelectRET(MachineInstruction *MI) override;
};

} // namespace RISCV