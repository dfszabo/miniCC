#include "TargetInstructionLegalizer.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include <cassert>

/// The following
///     MOD %res, %num, %mod
///
/// is replaced with
///     DIV %div_res, %num, %mod
///     MUL %mul_res, %div_res, %mod
///     SUB %res, %num, %mul_res
bool TargetInstructionLegalizer::ExpandMOD(MachineInstruction *MI,
                                           bool IsUnsigned) {
  assert(MI->GetOperandsNumber() == 3 && "MOD{U} must have exactly 3 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto ResVReg = *MI->GetOperand(0);
  auto NumVReg = *MI->GetOperand(1);
  auto ModVReg = *MI->GetOperand(2);

  assert(ResVReg.IsVirtualReg() && "Result must be a virtual register");
  assert(NumVReg.IsVirtualReg() && "Operand #1 must be a virtual register");
  assert((ModVReg.IsVirtualReg() || ModVReg.IsImmediate()) &&
         "Operand #2 must be a virtual register or an immediate");

  auto DIVResult = ParentFunc->GetNextAvailableVReg();
  auto DIV = MachineInstruction(IsUnsigned ? MachineInstruction::DIVU :
                                           MachineInstruction::DIV, ParentBB);
  DIV.AddOperand(MachineOperand::CreateVirtualRegister(DIVResult));
  DIV.AddOperand(NumVReg);
  DIV.AddOperand(ModVReg);
  auto DIVIter = ParentBB->ReplaceInstr(std::move(DIV), MI);

  auto MULResult = ParentFunc->GetNextAvailableVReg();
  auto MUL = MachineInstruction(MachineInstruction::MUL, ParentBB);
  MUL.AddOperand(MachineOperand::CreateVirtualRegister(MULResult));
  MUL.AddOperand(MachineOperand::CreateVirtualRegister(DIVResult));
  MUL.AddOperand(ModVReg);
  auto MULIter = ParentBB->InsertAfter(std::move(MUL), &*DIVIter);

  auto SUB = MachineInstruction(MachineInstruction::SUB, ParentBB);
  SUB.AddOperand(ResVReg);
  SUB.AddOperand(NumVReg);
  SUB.AddOperand(MachineOperand::CreateVirtualRegister(MULResult));
  ParentBB->InsertAfter(std::move(SUB), &*MULIter);

  return true;
}

/// If the target does not support storing directly an immediate, then
/// the following
///     STORE [address], immediate
///
/// is replaced with
///     LOAD_IMM %reg, immediate
///     STORE [address], %reg
///
/// where immediate is a constans number
bool TargetInstructionLegalizer::ExpandSTORE(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "STORE must have exactly 2 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Immediate = *MI->GetOperand(1);

  assert(Immediate.IsImmediate() && "Operand #2 must be an immediate");

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

bool TargetInstructionLegalizer::Expand(MachineInstruction *MI) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::MOD:
  case MachineInstruction::MODU:
    return ExpandMOD(MI, MI->GetOpcode() == MachineInstruction::MODU);
  case MachineInstruction::STORE:
    return ExpandSTORE(MI);
  case MachineInstruction::SUB:
    return ExpandSUB(MI);
  case MachineInstruction::MUL:
    return ExpandMUL(MI);
  case MachineInstruction::DIV:
    return ExpandDIV(MI);
  case MachineInstruction::DIVU:
    return ExpandDIVU(MI);
  case MachineInstruction::ZEXT:
    return ExpandZEXT(MI);
  case MachineInstruction::GLOBAL_ADDRESS:
    return ExpandGLOBAL_ADDRESS(MI);
  default:
    break;
  }

  return false;
}
