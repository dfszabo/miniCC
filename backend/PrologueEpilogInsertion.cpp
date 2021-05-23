#include "PrologueEpilogInsertion.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include "Support.hpp"

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
  MachineInstruction STR(MachineInstruction::STORE, nullptr);
  auto LROffset = Func.GetStackObjectPosition(TM->GetRegInfo()->GetLinkRegister());
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
  MachineInstruction LOAD(MachineInstruction::LOAD, nullptr);
  auto LROffset = Func.GetStackObjectPosition(TM->GetRegInfo()->GetLinkRegister());
  LROffset = GetNextAlignedValue(LROffset, 16);
  auto SPReg = TM->GetRegInfo()->GetStackRegister();
  auto Dest = TM->GetRegInfo()->GetLinkRegister();

  LOAD.AddRegister(Dest);
  LOAD.AddRegister(SPReg);
  LOAD.AddImmediate(LROffset);

  if (!TM->SelectInstruction(&LOAD))
    assert(!"Unable to select instruction");

  auto &LastBB = Func.GetBasicBlocks().back();
  LastBB.InsertInstr(LOAD, LastBB.GetInstructions().size() - 1);
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

  // NOTE: for now assuming that there is only one ret instruction and its
  // the last one of the last basic block, so we want to insert above it
  auto &LastBB = Func.GetBasicBlocks().back();
  LastBB.InsertInstr(ADDToSP, LastBB.GetInstructions().size() - 1);
}

void PrologueEpilogInsertion::Run() {
  for (auto &Func : MIRM->GetFunctions()) {
    if (Func.IsCaller())
      Func.GetStackFrame().InsertStackSlot(TM->GetRegInfo()->GetLinkRegister(), 16);

    // if there is no stack frame then do not emit adjustments
    if (Func.GetStackFrameSize() == 0)
      continue;

    InsertStackAdjustmentUpward(Func);

    if (Func.IsCaller()) {
      InsertLinkRegisterSave(Func);
      InsertLinkRegisterReload(Func);
    }
    /// TODO: handle spilled objects when spilling is implemented

    InsertStackAdjustmentDownward(Func);
  }
}