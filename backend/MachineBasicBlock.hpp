#ifndef MACHINEBASICBLOCK_HPP
#define MACHINEBASICBLOCK_HPP

#include "MachineInstruction.hpp"
#include <string>
#include <vector>

class MachineFunction;
class TargetMachine;

class MachineBasicBlock {
  using InstructionList = std::vector<MachineInstruction>;

public:
  MachineBasicBlock() = default;
  MachineBasicBlock(std::string &Name) : Name(Name) {}
  MachineBasicBlock(std::string &Name, MachineFunction *Parent)
      : Name(Name), Parent(Parent) {}

  std::string &GetName() { return Name; }
  InstructionList &GetInstructions() { return Instructions; }
  MachineFunction *GetParent() { return Parent; }

  /// Insert MI into back of the InstructionList
  void InsertInstr(MachineInstruction MI);

  /// Insert MI into InstructionList at Pos position
  InstructionList::iterator InsertInstr(MachineInstruction MI, size_t Pos);

  InstructionList::iterator InsertInstrToFront(MachineInstruction MI);

  /// Find where BeforeMI is in the InstructionList and insert MI before it
  InstructionList::iterator InsertBefore(MachineInstruction MI,
                                        MachineInstruction *BeforeMI);

  /// Find where BeforeMI is in the InstructionList and insert MIs before it
  InstructionList::iterator InsertBefore(std::vector<MachineInstruction> MIs,
                                         MachineInstruction *BeforeMI);

  /// Find where AfterMI is in the InstructionList and insert MI after it
  InstructionList::iterator InsertAfter(MachineInstruction MI,
                                        MachineInstruction *AfterMI);

  /// Find where AfterMI is in the InstructionList and insert MIs after it
  InstructionList::iterator InsertAfter(std::vector<MachineInstruction> MIs,
                                        MachineInstruction *AfterMI);

  /// Find where Replacable is in the InstructionList and replace it with MI
  InstructionList::iterator ReplaceInstr(MachineInstruction MI,
                                         MachineInstruction *Replacable);

  /// Return the instruction before MI
  MachineInstruction *GetPrecedingInstr(MachineInstruction *MI);

  /// Return the instruction after MI
  MachineInstruction *GetNextInstr(MachineInstruction *MI);

  void Erase(MachineInstruction *MI);

  void Print(TargetMachine *TM) const;

private:
  std::string Name;
  InstructionList Instructions;
  MachineFunction *Parent = nullptr;
};

#endif
