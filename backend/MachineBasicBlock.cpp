#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include <cassert>
#include <string>
#include <vector>
#include <iostream>

void MachineBasicBlock::InsertInstr(MachineInstruction MI) {
  if (MI.GetParent() == nullptr)
    MI.SetParent(this);
  Instructions.push_back(MI);
}

MachineBasicBlock::InstructionList::iterator
MachineBasicBlock::InsertInstr(MachineInstruction MI, size_t Pos) {
  if (MI.GetParent() == nullptr)
    MI.SetParent(this);
  return Instructions.insert(Instructions.begin() + Pos, MI);
}

MachineBasicBlock::InstructionList::iterator
MachineBasicBlock::InsertInstrToFront(MachineInstruction MI) {
  if (MI.GetParent() == nullptr)
    MI.SetParent(this);
  return Instructions.insert(Instructions.begin(), MI);
}

MachineBasicBlock::InstructionList::iterator
MachineBasicBlock::InsertAfter(MachineInstruction MI,
                               MachineInstruction *AfterMI) {
  size_t i = 0;
  for (; i < Instructions.size(); i++)
    if (&Instructions[i] == AfterMI)
      break;

  assert(i < Instructions.size() && "Instruction not found in the list");

  return InsertInstr(MI, i + 1);
}

MachineBasicBlock::InstructionList::iterator
MachineBasicBlock::ReplaceInstr(MachineInstruction MI,
                                MachineInstruction *Replacable) {
  size_t i = 0;
  for (; i < Instructions.size(); i++) {
    auto Ptr = &Instructions[i];
    if (Ptr == Replacable) {
      Instructions[i] = std::move(MI);
      return Instructions.begin() + i;
      break;
    }
  }
  assert(!"Replacable instruction was not found");
  return Instructions.end();
}

void MachineBasicBlock::Print() const {
  std::cout << "%BB:" << Name << ":" << std::endl;
  for (auto &I : Instructions) {
    std::cout << "\t";
    I.Print();
  }
}
