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

  virtual bool ExpandCMP(MachineInstruction *MI) { return false; }
  virtual bool ExpandMOD(MachineInstruction *MI, bool IsUnsigned);
  virtual bool ExpandSTORE(MachineInstruction *MI);
  virtual bool ExpandSUB(MachineInstruction *MI) { return false; }
  virtual bool ExpandMUL(MachineInstruction *MI) { return false; }
  virtual bool ExpandDIV(MachineInstruction *MI) { return false; }
  virtual bool ExpandDIVU(MachineInstruction *MI) { return false; }
  virtual bool ExpandZEXT(MachineInstruction *MI) { return false; }
  virtual bool ExpandGLOBAL_ADDRESS(MachineInstruction *MI) { return false; }

  /// Expanding the instruction into other ones which are compute the same
  /// value, but usually takes more instructions.
  bool Expand(MachineInstruction *MI);
};

#endif
