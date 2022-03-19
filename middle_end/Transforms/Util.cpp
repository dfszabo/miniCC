#include "Util.hpp"

void RenameRegisters(std::map<Value *, Value *> &Renameables,
                     BasicBlock::InstructionList &InstrList) {
  for (auto &I : InstrList) {
    if (I->IsStackAllocation() || I->IsJump())
      continue;

    if (I->Get1stUse() && Renameables.count(I->Get1stUse()))
      I->Set1stUse(Renameables[I->Get1stUse()]);

    if (I->Get2ndUse() && Renameables.count(I->Get2ndUse()))
      I->Set2ndUse(Renameables[I->Get2ndUse()]);
  }
}
