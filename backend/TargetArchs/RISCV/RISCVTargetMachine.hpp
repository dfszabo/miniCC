#include "../../MachineInstruction.hpp"
#include "../../TargetMachine.hpp"
#include "RISCVInstructionDefinitions.hpp"
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
  }

  ~RISCVTargetMachine() override {}

  bool SelectADD(MachineInstruction *MI) override;
  bool SelectMOD(MachineInstruction *MI) override;
  bool SelectCMP(MachineInstruction *MI) override;
  bool SelectLOAD(MachineInstruction *MI) override;
  bool SelectSTORE(MachineInstruction *MI) override;
  bool SelectBRANCH(MachineInstruction *MI) override;
  bool SelectJUMP(MachineInstruction *MI) override;
  bool SelectRET(MachineInstruction *MI) override;
};

} // namespace RISCV