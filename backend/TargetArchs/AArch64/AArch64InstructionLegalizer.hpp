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

  /// Since AArch64 does not support for immediate operand as 1st source operand
  /// for SUB (and for all arithmetic instruction as well), there for it has to
  /// be materialized first into a register
  /// TODO: expand the implementation for all arithmetic instruction
  bool ExpandSUB(MachineInstruction *MI) override;

  /// Since AArch64 do sign extension when loading therefore if the ZEXT is
  /// used only to zero extend a load result then it can be merged with the
  /// previous load into a ZEXT_LOAD.
  bool ExpandZEXT(MachineInstruction *MI) override;

  /// The global address materialization happens in two steps on arm. Example:
  ///   adrp x0, global_var
  ///   add  x0, x0, :lo12:global_var
  bool ExpandGLOBAL_ADDRESS(MachineInstruction *MI) override;
private:
  TargetMachine *TM = nullptr;
};

} // namespace AArch64

#endif
