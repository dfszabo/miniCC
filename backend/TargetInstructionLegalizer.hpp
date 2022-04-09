#ifndef TARGET_INSTRUCTION_LEGALIZER_HPP
#define TARGET_INSTRUCTION_LEGALIZER_HPP

#include "MachineInstruction.hpp"
#include <set>

class TargetMachine;

/// Instruction legalization interface for targets to implement if. If a target
/// does not support a target independent IR instruction, then it should create
/// a subclass of this class, where it specify which instruction is not legal
/// for the target.
class TargetInstructionLegalizer {
public:
  TargetInstructionLegalizer(TargetMachine *TM) : TM(TM) {}
  virtual ~TargetInstructionLegalizer() {}

  bool IsRelSupported(MachineInstruction::CMPRelation Rel) const;

  /// Predicate to decide which instructions are legal and which not.
  virtual bool Check(MachineInstruction *MI) { return true; }

  /// Predicate to decide whether the instruction is expandable or not.
  virtual bool IsExpandable(const MachineInstruction *MI) { return false; }

  virtual bool ExpandADD(MachineInstruction *MI);
  virtual bool ExpandADDS(MachineInstruction *MI);
  virtual bool ExpandADDC(MachineInstruction *MI);
  virtual bool ExpandXOR(MachineInstruction *MI);
  virtual bool ExpandCMP(MachineInstruction *MI);
  virtual bool ExpandMOD(MachineInstruction *MI, bool IsUnsigned);
  virtual bool ExpandLOAD(MachineInstruction *MI);
  virtual bool ExpandLOAD_IMM(MachineInstruction *MI);
  virtual bool ExpandSTORE(MachineInstruction *MI);
  virtual bool ExpandSUB(MachineInstruction *MI);
  virtual bool ExpandMUL(MachineInstruction *MI);
  virtual bool ExpandDIV(MachineInstruction *MI) { return false; }
  virtual bool ExpandDIVU(MachineInstruction *MI) { return false; }
  virtual bool ExpandZEXT(MachineInstruction *MI);
  virtual bool ExpandTRUNC(MachineInstruction *MI);
  virtual bool ExpandGLOBAL_ADDRESS(MachineInstruction *MI) { return false; }

  /// Expanding the instruction into other ones which are compute the same
  /// value, but usually takes more instructions.
  bool Expand(MachineInstruction *MI);
protected:
  TargetMachine *TM = nullptr;
  std::set<MachineInstruction::CMPRelation> UnSupportedRelations;
};

#endif
