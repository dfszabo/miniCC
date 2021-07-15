#include "AArch64InstructionLegalizer.hpp"
#include "AArch64InstructionDefinitions.hpp"
#include "../../MachineBasicBlock.hpp"
#include "../../MachineFunction.hpp"
#include "../../TargetMachine.hpp"
#include <cassert>

using namespace AArch64;

// Modulo operation is not legal on ARM, has to be expanded
bool AArch64InstructionLegalizer::Check(MachineInstruction *MI) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::MOD:
  case MachineInstruction::MODU:
    return false;
  case MachineInstruction::STORE:
    if (MI->GetOperands().back().IsImmediate())
      return false;
    break;
  case MachineInstruction::SUB:
    if (MI->GetOperand(1)->IsImmediate())
      return false;
    break;
  case MachineInstruction::MUL:
  case MachineInstruction::DIV:
  case MachineInstruction::DIVU:
    if (MI->GetOperand(2)->IsImmediate())
      return false;
    break;
  case MachineInstruction::ZEXT: {
    auto PrevInst = MI->GetParent()->GetPrecedingInstr(MI);

    // If not a LOAD then do nothing
    if (PrevInst && PrevInst->GetOpcode() == MachineInstruction::LOAD)
      return false;
    break;
  }
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
  case MachineInstruction::MODU:
  case MachineInstruction::STORE:
  case MachineInstruction::SUB:
  case MachineInstruction::MUL:
  case MachineInstruction::DIV:
  case MachineInstruction::DIVU:
  case MachineInstruction::ZEXT:
  case MachineInstruction::GLOBAL_ADDRESS:
    return true;

  default:
    break;
  }

  return false;
}

bool AArch64InstructionLegalizer::ExpandSTORE(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "STORE must have exactly 2 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Immediate = *MI->GetOperand(1);

  assert(Immediate.IsImmediate() && "Operand #2 must be an immediate");

  if (Immediate.GetImmediate() == 0) {
    MI->RemoveOperand(1);
    auto WZR = TM->GetRegInfo()->GetZeroRegister();
    MI->AddRegister(WZR, TM->GetRegInfo()->GetRegisterByID(WZR)->GetBitWidth());
    return true;
  }

  // Create the result register where the immediate will be loaded
  auto LOAD_IMMResult = ParentFunc->GetNextAvailableVReg();
  auto LOAD_IMMResultVReg =
      MachineOperand::CreateVirtualRegister(LOAD_IMMResult);

  // Replace the immediate operand with the result register
  MI->RemoveOperand(1);
  MI->AddOperand(LOAD_IMMResultVReg);

  MachineInstruction LOAD_IMM;
  LOAD_IMM.SetOpcode(MachineInstruction::LOAD_IMM);
  LOAD_IMM.AddOperand(LOAD_IMMResultVReg);
  LOAD_IMM.AddOperand(Immediate);
  ParentBB->InsertBefore(std::move(LOAD_IMM), MI);

  return true;
}

bool ExpandArithmeticInstWithImm(MachineInstruction *MI, size_t Index) {
  assert(MI->GetOperandsNumber() == 3 && "SUB must have exactly 3 operands");
  assert(Index < MI->GetOperandsNumber());
  auto ParentBB = MI->GetParent();

  auto MOV = MachineInstruction(MachineInstruction::LOAD_IMM, nullptr);
  auto DestReg = ParentBB->GetParent()->GetNextAvailableVReg();
  MOV.AddVirtualRegister(DestReg, MI->GetOperand(Index)->GetSize());
  MOV.AddOperand(*MI->GetOperand(Index));

  // replace the immediate operand with the destination of the immediate load
  MI->RemoveOperand(Index);
  MI->InsertOperand(Index,MachineOperand::CreateVirtualRegister(
      DestReg,MI->GetOperand(0)->GetSize()));

  // insert after modifying SUB, otherwise MI would became invalid
  ParentBB->InsertBefore(std::move(MOV), MI);

  return true;
}

bool AArch64InstructionLegalizer::ExpandSUB(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "SUB must have exactly 3 operands");
  return ExpandArithmeticInstWithImm(MI, 1);
}

bool AArch64InstructionLegalizer::ExpandMUL(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "MUL must have exactly 3 operands");
  return ExpandArithmeticInstWithImm(MI, 2);
}

bool AArch64InstructionLegalizer::ExpandDIV(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "DIV must have exactly 3 operands");
  return ExpandArithmeticInstWithImm(MI, 2);
}

bool AArch64InstructionLegalizer::ExpandDIVU(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "DIVU must have exactly 3 operands");
  return ExpandArithmeticInstWithImm(MI, 2);
}

bool AArch64InstructionLegalizer::ExpandZEXT(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "ZEXT must have exactly 2 operands");
  auto ParentBB = MI->GetParent();

  auto PrevInst = ParentBB->GetPrecedingInstr(MI);

  // If not a LOAD then do nothing
  if (!(PrevInst->GetOpcode() == MachineInstruction::LOAD))
    return false;

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
