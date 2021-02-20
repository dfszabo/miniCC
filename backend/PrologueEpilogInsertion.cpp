#include "PrologueEpilogInsertion.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include "Support.hpp"

MachineInstruction
PrologueEpilogInsertion::CreateADDInstruction(int64_t StackAdjustmentSize) {
  MachineInstruction Add(MachineInstruction::ADD, nullptr);

  auto SPReg = TM->GetRegInfo()->GetStackRegister();
  auto SPRegMO = MachineOperand::CreateRegister(SPReg);
  auto AdjustmentMO = MachineOperand::CreateImmediate(StackAdjustmentSize);

  Add.AddOperand(SPRegMO);
  Add.AddOperand(SPRegMO);
  Add.AddOperand(AdjustmentMO);

  if (!TM->SelectInstruction(&Add)) {
    assert(!"Unable to select instruction");
    return MachineInstruction();
  }

  return Add;
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
    InsertStackAdjustmentUpward(Func);
    /// TODO: handle spilled objects when spilling is implemented

    InsertStackAdjustmentDownward(Func);
  }
}