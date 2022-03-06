#include "PrologueEpilogInsertion.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include "Support.hpp"
#include "TargetInstruction.hpp"

/// TODO: Solve the stack issue: inserting physregs can collide with existing
/// stack slot with the same ID
static int NextStackSlot = 0;
std::map<unsigned, unsigned> LocalPhysRegToStackSlotMap;

MachineInstruction
PrologueEpilogInsertion::CreateADDInstruction(int64_t StackAdjustmentSize) {
  MachineInstruction Add(MachineInstruction::ADD, nullptr);

  auto SPReg = TM->GetRegInfo()->GetStackRegister();
  auto SPRegSize = TM->GetRegInfo()->GetRegisterByID(SPReg)->GetBitWidth();

  Add.AddRegister(SPReg, SPRegSize);
  Add.AddRegister(SPReg, SPRegSize);
  Add.AddImmediate(StackAdjustmentSize);

  if (!TM->SelectInstruction(&Add)) {
    assert(!"Unable to select instruction");
    return MachineInstruction();
  }

  return Add;
}

void PrologueEpilogInsertion::InsertLinkRegisterSave(MachineFunction &Func) {
  if (!Func.IsCaller())
    return;

  MachineInstruction STR(MachineInstruction::STORE, nullptr);
  auto LROffset = Func.GetStackObjectPosition(
      LocalPhysRegToStackSlotMap[TM->GetRegInfo()->GetLinkRegister()]);
  LROffset = GetNextAlignedValue(LROffset, 16);
  auto SPReg = TM->GetRegInfo()->GetStackRegister();
  auto Dest = TM->GetRegInfo()->GetLinkRegister();

  STR.AddRegister(Dest);
  STR.AddRegister(SPReg);
  STR.AddImmediate(LROffset);

  if (!TM->SelectInstruction(&STR))
    assert(!"Unable to select instruction");

  // Stack adjusting instruction already inserted at this point so this have
  // to be inserted after it, hence the position 1
  Func.GetBasicBlocks().front().InsertInstr(STR, 1);
}

void PrologueEpilogInsertion::InsertLinkRegisterReload(MachineFunction &Func) {
  if (!Func.IsCaller())
    return;

  MachineInstruction LOAD(MachineInstruction::LOAD, nullptr);
  auto LROffset = Func.GetStackObjectPosition(
      LocalPhysRegToStackSlotMap[TM->GetRegInfo()->GetLinkRegister()]);
  LROffset = GetNextAlignedValue(LROffset, 16);
  auto SPReg = TM->GetRegInfo()->GetStackRegister();
  auto Dest = TM->GetRegInfo()->GetLinkRegister();

  LOAD.AddRegister(Dest);
  LOAD.AddRegister(SPReg);
  LOAD.AddImmediate(LROffset);

  if (!TM->SelectInstruction(&LOAD))
    assert(!"Unable to select instruction");

  auto &RetBB = Func.GetBasicBlocks()[MBBWithRetIdx];
  RetBB.InsertInstr(LOAD,
                     std::max(0, (int)RetBB.GetInstructions().size() - 1));
}

void PrologueEpilogInsertion::InsertStackAdjustmentUpward(
    MachineFunction &Func) {
  unsigned StackAlignment = TM->GetABI()->GetStackAlignment();

  int64_t StackAdjustmentSize =
      GetNextAlignedValue(Func.GetStackFrameSize(), StackAlignment) * -1;

  MachineInstruction ADDToSP = CreateADDInstruction(StackAdjustmentSize);

  Func.GetBasicBlocks().front().InsertInstrToFront(ADDToSP);
}

void PrologueEpilogInsertion::InsertStackAdjustmentDownward(
    MachineFunction &Func) {
  unsigned StackAlignment = TM->GetABI()->GetStackAlignment();

  int64_t StackAdjustmentSize =
      GetNextAlignedValue(Func.GetStackFrameSize(), StackAlignment);

  MachineInstruction ADDToSP = CreateADDInstruction(StackAdjustmentSize);

  auto &RetBB = Func.GetBasicBlocks()[MBBWithRetIdx];
  assert(RetBB.GetInstructions().size() > 0);
  RetBB.InsertInstr(ADDToSP, RetBB.GetInstructions().size() - 1);
}

MachineInstruction PrologueEpilogInsertion::CreateSTORE(MachineFunction &Func,
                                                        unsigned Register) {
  MachineInstruction STR(MachineInstruction::STORE, nullptr);
  auto StackID = Register;
  if (LocalPhysRegToStackSlotMap.count(Register) > 0)
    StackID = LocalPhysRegToStackSlotMap[Register];
  auto Offset = Func.GetStackObjectPosition(StackID);
  Offset = GetNextAlignedValue(Offset, TM->GetPointerSize() / 8);
  auto SPReg = TM->GetRegInfo()->GetStackRegister();

  STR.AddRegister(Register, TM->GetPointerSize());
  STR.AddRegister(SPReg);
  STR.AddImmediate(Offset);

  if (!TM->SelectInstruction(&STR))
    assert(!"Unable to select instruction");

  return STR;
}

MachineInstruction PrologueEpilogInsertion::CreateLOAD(MachineFunction &Func,
                                                        unsigned Register) {
  MachineInstruction LOAD(MachineInstruction::LOAD, nullptr);
  auto StackID = Register;
  if (LocalPhysRegToStackSlotMap.count(Register) > 0)
    StackID = LocalPhysRegToStackSlotMap[Register];
  auto Offset = Func.GetStackObjectPosition(StackID);
  Offset = GetNextAlignedValue(Offset, TM->GetPointerSize() / 8);
  auto SPReg = TM->GetRegInfo()->GetStackRegister();

  LOAD.AddRegister(Register, TM->GetPointerSize());
  LOAD.AddRegister(SPReg);
  LOAD.AddImmediate(Offset);

  if (!TM->SelectInstruction(&LOAD))
    assert(!"Unable to select instruction");

  return LOAD;
}

void PrologueEpilogInsertion::SpillClobberedCalleeSavedRegisters(
    MachineFunction &Func) {
  unsigned Counter = 0;
  const unsigned StartOfInsertion = Func.IsCaller() ? 2 : 1;

  for (auto Reg : Func.GetUsedCalleSavedRegs()) {
    auto STR = CreateSTORE(Func, Reg);
    Func.GetBasicBlocks().front().InsertInstr(STR, StartOfInsertion + Counter);
    Counter++;
  }
}

void PrologueEpilogInsertion::ReloadClobberedCalleeSavedRegisters(
    MachineFunction &Func) {
  unsigned Counter = 0;
  auto &RetBB = Func.GetBasicBlocks()[MBBWithRetIdx];

  for (auto Reg : Func.GetUsedCalleSavedRegs()) {
    auto LOAD = CreateLOAD(Func, Reg);
    RetBB.InsertInstr(LOAD, RetBB.GetInstructions().size() - 1 - Counter);
    Counter++;
  }
}

void PrologueEpilogInsertion::Run() {
  for (auto &Func : MIRM->GetFunctions()) {
    // if there is no stack frame then do not emit adjustments
    if (Func.GetStackFrameSize() == 0 && Func.GetUsedCalleSavedRegs().empty())
      continue;

    // reset state before processing a new function
    LocalPhysRegToStackSlotMap.clear();
    MBBWithRetIdx = ~0;
    NextStackSlot = 10000;

    for (auto CalleSavedReg : Func.GetUsedCalleSavedRegs()) {
      Func.GetStackFrame().InsertStackSlot(NextStackSlot,
                                           TM->GetPointerSize() / 8,
                                           TM->GetPointerSize() / 8);
      LocalPhysRegToStackSlotMap[CalleSavedReg] = NextStackSlot++;
    }

    if (Func.IsCaller()) {
      Func.GetStackFrame().InsertStackSlot(NextStackSlot, TM->GetPointerSize() / 8, 16);
      LocalPhysRegToStackSlotMap[TM->GetRegInfo()->GetLinkRegister()] = NextStackSlot++;
    }

    // find where the ret is
    for (size_t i = 0; i < Func.GetBasicBlocks().size(); i++) {
      for (auto &MI : Func.GetBasicBlocks()[i].GetInstructions())
        if (TM->GetInstrDefs()->GetTargetInstr(MI.GetOpcode())->IsReturn()) {
          MBBWithRetIdx = i;
          break;
        }
      // break out from the outer loop aswell
      if (MBBWithRetIdx != ~0u)
        break;
    }

    assert(MBBWithRetIdx != ~0u && "Have not found a return instruction");

    InsertStackAdjustmentUpward(Func);
    InsertLinkRegisterSave(Func);
    SpillClobberedCalleeSavedRegisters(Func);
    ReloadClobberedCalleeSavedRegisters(Func);
    InsertLinkRegisterReload(Func);
    InsertStackAdjustmentDownward(Func);
  }
}