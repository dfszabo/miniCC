#include "CSEPass.hpp"
#include "../IR/BasicBlock.hpp"
#include "../IR/Function.hpp"
#include "Util.hpp"
#include <map>

struct AliveDefinitions {
  std::vector<Instruction *> Instructions;

  AliveDefinitions() = default;

  void InvalidateAll() { Instructions.clear(); }

  void InsertDef(Instruction *I) { Instructions.push_back(I); }

  Instruction *GetAlreadyComputedExpression(Instruction *I) {
    for (auto Instr : Instructions) {
      // If there is already an instruction with the same type, which used the
      // same operands
      if (I->GetInstructionKind() == Instr->GetInstructionKind() &&
          I->Get1stUse() == Instr->Get1stUse() &&
          I->Get2ndUse() == Instr->Get2ndUse()) {
        // then the value computed by I is already computed, return it's
        // defining instruction
        return Instr;
      }
    }

    return nullptr;
  }
};

static void ProcessBB(std::unique_ptr<BasicBlock> &BB) {
  auto &InstList = BB->GetInstructions();
  std::map<Value *, Value *> Renamables;
  AliveDefinitions AliveDefs;

  for (auto &Instr : InstList) {
    auto InstrPtr = Instr.get();

    // Nothing to do with stack allocations or jumps. Also it is assumed, that
    // copy propagation was already done before this pass, therefore loads can
    // also be ignored.
    if (InstrPtr->IsStackAllocation() || InstrPtr->IsJump())
      continue;

    // call -s might clobber registers at the target level, so anything defined
    // before a call will be invalid. Although this is IR level and should not
    // care about this here, but for now it is just easier to do it now.
    // TODO: fix it
    if (InstrPtr->IsCall()) {
      AliveDefs.InvalidateAll();
      continue;
    }

    // If the current instruction computation is already done by previous
    // ones then register it as renamable
    if (auto ACE = AliveDefs.GetAlreadyComputedExpression(InstrPtr)) {
      Renamables[InstrPtr] = ACE;
    } else {
      // Otherwise it is a newly defined expression/value, so register it
      AliveDefs.InsertDef(InstrPtr);
    }
  }

  if (!Renamables.empty())
    RenameRegisters(Renamables, InstList);
}

bool CSEPass::RunOnFunction(Function &F) {
  for (auto &BB : F.GetBasicBlocks())
    ProcessBB(BB);

  return true;
}
