#ifndef AARCH64_INSTRUCTION_LEGALIZER_HPP
#define AARCH64_INSTRUCTION_LEGALIZER_HPP

#include "../../MachineInstruction.hpp"
#include "../../TargetInstructionLegalizer.hpp"

namespace AArch64 {

class AArch64InstructionLegalizer : public TargetInstructionLegalizer {
public:
  AArch64InstructionLegalizer() {}
  ~AArch64InstructionLegalizer() override {}

  bool Check(MachineInstruction *MI) override;
  bool IsExpandable(const MachineInstruction *MI) override;

private:
};

} // namespace AArch64

#endif
