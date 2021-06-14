#ifndef TARGET_INSTRUCTION_LEGALIZER_HPP
#define TARGET_INSTRUCTION_LEGALIZER_HPP

class MachineInstruction;

/// Instruction legalization interface for targets to implement if. If a target
/// does not support a target independent IR instruction, then it should create
/// a subclass of this class, where it specify which instruction is not legal
/// for the target.
class TargetInstructionLegalizer {
public:
  TargetInstructionLegalizer() {}
  virtual ~TargetInstructionLegalizer() {}

  /// Predicate to decide which instructions are legal and which not.
  virtual bool Check(MachineInstruction *MI) { return true; }

  /// Predicate to decide whether the instruction is expandable or not.
  virtual bool IsExpandable(const MachineInstruction *MI) { return false; }

  virtual bool ExpandMOD(MachineInstruction *MI);
  virtual bool ExpandSTORE(MachineInstruction *MI);
  virtual bool ExpandGLOBAL_ADDRESS(MachineInstruction *MI) { return false; }

  /// Expanding the instruction into other ones which are compute the same
  /// value, but usually takes more instructions.
  bool Expand(MachineInstruction *MI);
};

#endif
