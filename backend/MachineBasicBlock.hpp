#ifndef MACHINEBASICBLOCK_HPP
#define MACHINEBASICBLOCK_HPP

#include "MachineInstruction.hpp"
#include <string>
#include <vector>

class MachineFunction;

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

  /// Find where AfterMI is in the InstructionList and insert MI after it
  InstructionList::iterator InsertAfter(MachineInstruction MI,
                                        MachineInstruction *AfterMI);

  /// Find where Replacable is in the InstructionList and replace it with MI
  InstructionList::iterator ReplaceInstr(MachineInstruction MI,
                                         MachineInstruction *Replacable);

  void Print() const;

private:
  std::string Name;
  InstructionList Instructions;
  MachineFunction *Parent = nullptr;
};

#endif
