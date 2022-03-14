#include "BasicBlock.hpp"
#include "Instructions.hpp"

Instruction *BasicBlock::Insert(std::unique_ptr<Instruction> Instruction) {
  Instructions.push_back(std::move(Instruction));
  return Instructions.back().get();
}

Instruction *BasicBlock::InsertSA(std::unique_ptr<Instruction> Instruction) {
  // Edge case: The BB is empty -> Just use Insert
  if (Instructions.empty())
    return Insert(std::move(Instruction));

  size_t i;
  for (i = 0; i < Instructions.size(); i++)
    if (!Instructions[i]->IsStackAllocation()) {
      auto InstPtr = Instruction.get();
      Instructions.insert(Instructions.begin() + i, std::move(Instruction));
      return InstPtr;
    }

  // Edge case: Every instruction were stack allocations -> Just use Insert
  return Insert(std::move(Instruction));
}

void BasicBlock::Print() const {
  std::cout << "." << Name << ":" << std::endl;
  for (auto &Instruction : Instructions)
    Instruction->Print();
}
