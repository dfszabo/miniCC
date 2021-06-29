#include "AArch64InstructionLegalizer.hpp"
#include "AArch64InstructionDefinitions.hpp"
#include "../../MachineBasicBlock.hpp"
#include "../../MachineFunction.hpp"
#include <cassert>

using namespace AArch64;

// Modulo operation is not legal on ARM, has to be expanded
bool AArch64InstructionLegalizer::Check(MachineInstruction *MI) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::MOD:
    return false;
  case MachineInstruction::STORE:
    assert(MI->GetOperandsNumber() == 2 && "Must have 2 operands");
    if (MI->GetOperand(1)->IsImmediate())
      return false;
    break;
  case MachineInstruction::ZEXT:
  case MachineInstruction::GLOBAL_ADDRESS:
    return false;
  default:
    break;
  }

  return true;
}

bool AArch64InstructionLegalizer::IsExpandable(const MachineInstruction *MI) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::MOD:
  case MachineInstruction::STORE:
  case MachineInstruction::ZEXT:
  case MachineInstruction::GLOBAL_ADDRESS:
    return true;

  default:
    break;
  }

  return false;
}

bool AArch64InstructionLegalizer::ExpandZEXT(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "ZEXT must have exactly 2 operands");
  auto ParentBB = MI->GetParent();

  auto PrevInst = ParentBB->GetPrecedingInstr(MI);

  // If not a LOAD then do nothing
  if (!(PrevInst->GetOpcode() == MachineInstruction::LOAD))
    return true;

  auto ZEXTDest = *MI->GetOperand(0);
  PrevInst->InsertOperand(0, ZEXTDest);
  PrevInst->SetOpcode(MachineInstruction::ZEXT_LOAD);
  ParentBB->Erase(MI);

  return true;
}

bool AArch64InstructionLegalizer::ExpandGLOBAL_ADDRESS(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "GLOBAL_ADDRESS must have exactly 2 operands");
  auto ParentBB = MI->GetParent();

  auto GlobalVar = *MI->GetOperand(1);
  assert(GlobalVar.IsGlobalSymbol() && "Operand #2 must be a symbol");
  auto GlobalVarName = ":lo12:" + GlobalVar.GetGlobalSymbol();

  MI->SetOpcode(ADRP);

  MachineInstruction ADD;
  ADD.SetOpcode(MachineInstruction::ADD);
  auto DestReg = *MI->GetOperand(0);
  ADD.AddOperand(DestReg);
  ADD.AddOperand(DestReg);
  ADD.AddGlobalSymbol(GlobalVarName);
  ParentBB->InsertAfter(std::move(ADD), MI);

  return true;
}
