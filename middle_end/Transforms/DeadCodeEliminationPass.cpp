#include "DeadCodeEliminationPass.hpp"
#include "../IR/BasicBlock.hpp"
#include "../IR/Function.hpp"
#include "../IR/Instructions.hpp"
#include <set>

void FindDeadInstructions(const std::unique_ptr<BasicBlock> &BB,
                          std::vector<size_t> &DeadInstrIndexes) {
  auto &Instructions = BB->GetInstructions();
  std::set<Value *> UsedValues;

  for (int i = Instructions.size() - 1; i >= 0; i--) {
    if (auto Use1 = Instructions[i]->Get1stUse(); Use1 && Use1->IsRegister())
      UsedValues.insert(Use1);

    if (auto Use2 = Instructions[i]->Get2ndUse(); Use2 && Use2->IsRegister())
      UsedValues.insert(Use2);

    // If it is a call then added all of it's parameters to the use set
    if (Instructions[i]->IsCall())
      for (auto Param :
           dynamic_cast<CallInstruction *>(Instructions[i].get())->GetArgs())
        UsedValues.insert(Param);

    // if an instruction does not define a value then it considered alive
    // also stack allocation and calls too
    if (!Instructions[i]->IsDef() || Instructions[i]->IsStackAllocation() ||
        Instructions[i]->IsCall())
      continue;

    // If the instruction result has no uses after it (note: iteration is bottom
    // up) then it's defined value is dead, mark it for termination.
    if (UsedValues.count(Instructions[i].get()) == 0)
      DeadInstrIndexes.push_back(i);
  }
}

// Deleting the instructions at the indexes defined by @DeadInstrIndexes.
// It is assumed that @DeadInstrIndexes is sorted in descending order, which
// should be true given that FindDeadInstructions doing a bottom up scan of
// the instructions.
void DeleteInstructions(const std::unique_ptr<BasicBlock> &BB,
                        std::vector<size_t> &DeadInstrIndexes) {
  auto &Instructions = BB->GetInstructions();

  for (auto Index : DeadInstrIndexes)
    Instructions.erase(Instructions.begin() + Index);
}

bool DeadCodeEliminationPass::RunOnFunction(Function &F) {
  std::vector<size_t> DeadInstrIndexes;
  for (auto &BB : F.GetBasicBlocks()) {
    DeadInstrIndexes.clear();
    auto &Instructions = BB->GetInstructions();

    FindDeadInstructions(BB, DeadInstrIndexes);
    if (!DeadInstrIndexes.empty())
      DeleteInstructions(BB, DeadInstrIndexes);

    for (size_t i = 0; i < Instructions.size(); i++) {
      // If a basic block terminator instruction has been found AND it is a JUMP
      // AND after it there is no ret instruction, then delete the remaining
      // instruction from this BB, since they are dead code.
      if (Instructions[i]->IsTerminator() && i + 1 < Instructions.size()) {
        bool CanDeleteTheRest = true;

        if (Instructions[i]->IsJump())
          for (auto It = Instructions.begin() + i + 1; It != Instructions.end();
               It++)
            if (It->get()->IsReturn()) {
              CanDeleteTheRest = false;
              break;
            }

        if (CanDeleteTheRest)
          Instructions.erase(Instructions.begin() + i + 1, Instructions.end());
        break;
      }
    }
  }

  return false;
}
