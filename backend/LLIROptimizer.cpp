#include "LLIROptimizer.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineIRModule.hpp"
#include "TargetMachine.hpp"

struct AliveDefinitions {
  std::vector<MachineInstruction *> Instructions;

  AliveDefinitions() = default;

  void InvalidateAll() { Instructions.clear(); }

  void InsertDef(MachineInstruction *I) { Instructions.push_back(I); }

  MachineOperand *GetAlreadyComputedExpression(MachineInstruction &I) {
    for (auto Instr : Instructions) {
      // If there is already an instruction with the same type, which used the
      // same operands
      if (I.GetOpcode() != Instr->GetOpcode())
        return nullptr;

      for (int i = 0; i < 2; i++) {
        if (!I.GetNthUse(i) || !Instr->GetNthUse(i))
          continue;

        MachineOperand L = *I.GetNthUse(i);
        MachineOperand R = *Instr->GetNthUse(i);
        if (L != R)
          return nullptr;
      }
      // then the value computed by I is already computed, return it's
      // defining instruction
      return Instr->GetDef();
    }

    return nullptr;
  }
};

static void RenameRegisters(std::map<uint64_t, uint64_t> &Renameables,
                            MachineBasicBlock::InstructionList &InstrList) {
  for (auto &I : InstrList) {
    if (I.IsJump())
      continue;

    for (int i = 0; i < 2; i++) {
      auto Use = I.GetNthUse(i);
      const bool IsVirtualRegType =
          Use && (Use->IsVirtualReg() || (Use->IsMemory() && Use->IsVirtual()));
      if (IsVirtualRegType && Renameables.count(Use->GetReg()))
        Use->SetReg(Renameables[Use->GetReg()]);
    }
  }
}

static void CopyPropagation(MachineBasicBlock &MBB) {
  auto &InstList = MBB.GetInstructions();
  std::map<uint64_t, uint64_t> Renamables;
  std::map<uint64_t, uint64_t> KnownMemoryValues; // stack slot to vreg

  for (int i = 0; i < (int)InstList.size(); i++) {
    auto Instr = InstList[i];

    // call -s will clobber registers, so anything defined before a call will
    // might be invalid.
    if (Instr.IsCall()) {
      KnownMemoryValues.clear();
      continue;
    }

    // If it is a load from a stack allocation, then register
    // the source as a know value, since it's value is now held in the
    // destination of the load.
    if (Instr.IsLoad()) {
      auto Source = Instr.GetNthUse(0);

      // If the source is null or the load use an offset or it is not a stack
      // access then skip it
      if (!Source || Source->GetOffset() != 0 || !Source->IsStackAccess())
        continue;

      // Only checking virtual regs
      if (!Instr.GetDef()->IsVirtualReg())
        continue;

      // If the value is not known yet, then it is now
      if (KnownMemoryValues.count(Source->GetSlot()) == 0) {
        assert(Instr.GetDef()->IsVirtualReg());
        KnownMemoryValues[Source->GetSlot()] = Instr.GetDef()->GetReg();
      }
      // Otherwise there is already a load or store which defined this
      // stack allocation or global variable, therefore it is known and this
      // load is superflous. Register it's definition as renamable.
      else
        Renamables[Instr.GetDef()->GetReg()] =
            KnownMemoryValues[Source->GetSlot()];
    }

    // Similarly as load, if a value is stored to a stack allocation, then that
    // value is known, therefore does not require load in subsequent uses.
    else if (Instr.IsStore()) {
      auto Source = Instr.GetNthUse(1);

      // If source is null or the stored value is not a register, then skip it
      if (!Source || !Source->IsVirtualReg())
        continue;

      // Only checking stack slots
      if (!Instr.GetNthUse(0) || Instr.GetNthUse(0)->GetOffset() != 0 ||
          !Instr.GetNthUse(0)->IsStackAccess())
        continue;

      KnownMemoryValues[Instr.GetNthUse(0)->GetSlot()] = Source->GetReg();
    }
  }

  if (!Renamables.empty())
    RenameRegisters(Renamables, InstList);
}

static void CSE(MachineBasicBlock &MBB) {
  auto &InstList = MBB.GetInstructions();
  std::map<uint64_t, uint64_t> Renamables;
  AliveDefinitions AliveDefs;

  for (int i = 0; i < (int)InstList.size(); i++) {
    auto Instr = InstList[i];

    // Nothing to do with stack allocations or jumps. Also it is assumed, that
    // copy propagation was already done before this pass, therefore loads can
    // also be ignored.
    if (Instr.IsJump())
      continue;

    // call -s might clobber registers at the target level, so anything defined
    // before a call will be invalid. Although this is IR level and should not
    // care about this here, but for now it is just easier to do it now.
    // TODO: fix it
    if (Instr.IsCall()) {
      AliveDefs.InvalidateAll();
      continue;
    }

    // If the current instruction computation is already done by previous
    // ones then register it as renamable
    if (auto ACE = AliveDefs.GetAlreadyComputedExpression(Instr)) {
      Renamables[Instr.GetDef()->GetReg()] = ACE->GetReg();
    } else {
      // Otherwise it is a newly defined expression/value, so register it
      AliveDefs.InsertDef(&InstList[i]);
    }
  }

  if (!Renamables.empty())
    RenameRegisters(Renamables, InstList);
}

static void DeadCodeElimination(MachineBasicBlock &MBB) {
  auto &Instructions = MBB.GetInstructions();
  std::set<uint64_t> UsedValues;

  for (int i = Instructions.size() - 1; i >= 0; i--) {
    if (auto Use1 = Instructions[i].GetNthUse(0);
        Use1 &&
        (Use1->IsVirtualReg() || (Use1->IsMemory() && Use1->IsVirtual())))
      UsedValues.insert(Use1->GetReg());

    if (auto Use2 = Instructions[i].GetNthUse(1);
        Use2 &&
        (Use2->IsVirtualReg() || (Use2->IsMemory() && Use2->IsVirtual())))
      UsedValues.insert(Use2->GetReg());

    // if an instruction does not define a value then it considered alive
    // also calls too and skip instructions which already has allocated def regs
    // these typically are instructions that preparing the parameters for a call
    if (!Instructions[i].IsDef() || Instructions[i].IsCall() ||
        Instructions[i].GetDef()->IsRegister())
      continue;

    // If the instruction result has no uses after it (note: iteration is bottom
    // up) then it's defined value is dead, mark it for termination.
    if (UsedValues.count(Instructions[i].GetDef()->GetReg()) == 0)
      Instructions.erase(Instructions.begin() + i);
  }
}

void LLIROptimizer::Run() {
  for (auto &MFunc : MIRM->GetFunctions()) {
    for (auto &MBB : MFunc.GetBasicBlocks()) {
      CopyPropagation(MBB);
      DeadCodeElimination(MBB);
      CSE(MBB);
      DeadCodeElimination(MBB);
    }
  }
}
