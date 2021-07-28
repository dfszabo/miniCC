#ifndef AARCH64_INSTRUCTION_LEGALIZER_HPP
#define AARCH64_INSTRUCTION_LEGALIZER_HPP

#include "../../MachineInstruction.hpp"
#include "../../TargetInstructionLegalizer.hpp"

namespace AArch64 {

class AArch64InstructionLegalizer : public TargetInstructionLegalizer {
public:
  AArch64InstructionLegalizer(TargetMachine *TM) : TM(TM) {}
  ~AArch64InstructionLegalizer() override {}

  bool Check(MachineInstruction *MI) override;
  bool IsExpandable(const MachineInstruction *MI) override;

  /// Use wzr register if the stored immediate is 0
  bool ExpandSTORE(MachineInstruction *MI) override;

  /// Since AArch64 does not support for immediate operand as first source
  /// operand for SUB (and for other arithmetic instruction as well), therefore
  /// it has to be materialized first into a register
  bool ExpandSUB(MachineInstruction *MI) override;

  /// Since AArch64 does not support for immediate operand as last source
  /// operand for some arithmetic instruction, therefore it has to be
  /// materialized first into a register
  bool ExpandMUL(MachineInstruction *MI) override;
  bool ExpandDIV(MachineInstruction *MI) override;
  bool ExpandDIVU(MachineInstruction *MI) override;

  /// The global address materialization happens in two steps on arm. Example:
  ///   adrp x0, global_var
  ///   add  x0, x0, :lo12:global_var
  bool ExpandGLOBAL_ADDRESS(MachineInstruction *MI) override;
private:
  TargetMachine *TM = nullptr;
};

} // namespace AArch64

#endif
