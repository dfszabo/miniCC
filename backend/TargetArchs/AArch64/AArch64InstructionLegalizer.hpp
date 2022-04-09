#ifndef AARCH64_INSTRUCTION_LEGALIZER_HPP
#define AARCH64_INSTRUCTION_LEGALIZER_HPP

#include "../../MachineInstruction.hpp"
#include "../../TargetInstructionLegalizer.hpp"

namespace AArch64 {

class AArch64InstructionLegalizer : public TargetInstructionLegalizer {
public:
  AArch64InstructionLegalizer(TargetMachine *TM)
      : TargetInstructionLegalizer(TM) {}
  ~AArch64InstructionLegalizer() override {}

  bool Check(MachineInstruction *MI) override;
  bool IsExpandable(const MachineInstruction *MI) override;

  /// If the CMP result is NOT used for a subsequent jump, then a CSET
  /// must be issued to materialize the compare result into an actual GPR.
  /// example IR:
  ///         cmp.eq  $1<i1>, $2<u32>, 420<u32>
  ///         ret     $2<i1>
  ///
  /// here the comparison result should be returned, but the cmp.xx is
  /// selected to cmp, which only set an implicit register, so the above
  /// code will results in (maybe with other non w0 registers):
  ///         cmp     w1, w2
  ///         ret
  ///
  /// the cmp set the implicit compare result register, but thats not enough,
  /// the result should be in the return register. Therefore the following
  /// sequence is desired:
  ///         cmp     w1, w2
  ///         cset    w0, eq
  ///         ret
  bool ExpandCMP(MachineInstruction *MI) override;

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
};

} // namespace AArch64

#endif
