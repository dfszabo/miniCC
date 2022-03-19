#include "CopyPropagationPass.hpp"
#include "../IR/BasicBlock.hpp"
#include "../IR/Function.hpp"
#include "Util.hpp"
#include <map>

static void ProcessBB(std::unique_ptr<BasicBlock> &BB) {
  auto &InstList = BB->GetInstructions();
  std::map<Value *, Value *> Renamables;
  std::map<Value *, Instruction *> KnownMemoryValues;

  for (auto &Instr : InstList) {
    auto InstrPtr = Instr.get();

    // call -s will clobber registers at the target level, so anything defined
    // before a call will be invalid. Although this is IR level and should not
    // care about this here, but for it is just easier to do it now.
    // TODO: fix it
    if (InstrPtr->IsCall()) {
      KnownMemoryValues.clear();
      continue;
    }

    // If it is a load from a stack allocation or global variable, then register
    // the source as a know value, since it's value is now held in the
    // destination of the load.
    if (InstrPtr->IsLoad()) {
      auto Source = InstrPtr->Get1stUse();

      // If the source is null or the load use an offset then skip it
      if (!Source || InstrPtr->Get2ndUse())
        continue;

      // Only checking global vars and stack allocations
      if (!Source->IsGlobalVar() &&
          !dynamic_cast<StackAllocationInstruction *>(Source))
        continue;

      // If the value is not known yet, then it is now
      if (KnownMemoryValues.count(Source) == 0)
        KnownMemoryValues[Source] = InstrPtr;
      // Otherwise there is already a load or store which defined this
      // stack allocation or global variable, therefore it is known and this
      // load is superflous. Register it's definition as renamable.
      else
        Renamables[InstrPtr] = KnownMemoryValues[Source];
    }

    // Similarly as load, if a value is stored to a stack allocation or global
    // var, then that value is known, therefore does not require load in
    // subsequent uses.
    else if (InstrPtr->IsStore()) {
      auto Source = InstrPtr->Get2ndUse();

      // If source is null or the stored value is not a register, then skip it
      if (!Source || !InstrPtr->Get1stUse()->IsRegister())
        continue;

      // Only checking global vars and stack allocations
      if (!Source->IsGlobalVar() &&
          !dynamic_cast<StackAllocationInstruction *>(Source))
        continue;

      KnownMemoryValues[Source] =
          dynamic_cast<Instruction *>(InstrPtr->Get1stUse());
    }
  }

  if (!Renamables.empty())
    RenameRegisters(Renamables, InstList);
}

bool CopyPropagationPass::RunOnFunction(Function &F) {
  for (auto &BB : F.GetBasicBlocks())
    ProcessBB(BB);

  return true;
}
