#include "Function.hpp"
#include "BasicBlock.hpp"
#include "Type.hpp"
#include <cassert>
#include <iostream>

Function::Function(const std::string &Name, IRType RT)
    : Name(Name), ReturnType(RT) {
  auto FinalName = std::string("entry_") + Name;
  auto Ptr = new BasicBlock(FinalName, this);
  std::unique_ptr<BasicBlock> BB(Ptr);
  BasicBlocks.push_back(std::move(BB));
}

BasicBlock *Function::GetCurrentBB() {
  assert(BasicBlocks.size() > 0 && "Function must have basic blocks.");
  return BasicBlocks.back().get();
}

BasicBlock *Function::GetBB(const size_t Index) {
  assert(BasicBlocks.size() > 0 && "Function must have basic blocks.");
  assert(BasicBlocks.size() > Index && "Invalid index.");
  return BasicBlocks[Index].get();
}

void Function::CreateBasicBlock() {
  auto BB = std::make_unique<BasicBlock>(BasicBlock(this));
  BasicBlocks.push_back(std::move(BB));
}

void Function::Insert(std::unique_ptr<BasicBlock> BB) {
  BasicBlocks.push_back(std::move(BB));
}

void Function::Insert(std::unique_ptr<FunctionParameter> FP) {
  Parameters.push_back(std::move(FP));
}

void Function::Print() const {
  std::cout << "func " << Name << " (";

  auto size = Parameters.size();
  if (!(size == 1 && Parameters[0]->GetType().IsVoid())) {
    for (unsigned i = 0; i < size; i++) {
      std::cout << Parameters[i]->ValueString() << " :";
      std::cout << Parameters[i]->GetType().AsString();
      if (i + 1 < size)
        std::cout << ", ";
    }
  }

  std::cout << ")";
  if (!ReturnType.IsVoid()) {
    std::cout << " -> " << ReturnType.AsString();
  }
  std::cout << ":" << std::endl;

  for (auto &BB : BasicBlocks)
    BB->Print();

  std::cout << std::endl;
}
