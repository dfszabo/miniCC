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
bool ExpandMOD(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "MOD must have exactly 3 operands");
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
  MachineInstruction DIV;
  DIV.SetOpcode(MachineInstruction::DIV);
  DIV.AddOperand(MachineOperand::CreateVirtualRegister(DIVResult));
  DIV.AddOperand(NumVReg);
  DIV.AddOperand(ModVReg);
  auto DIVIter = ParentBB->ReplaceInstr(std::move(DIV), MI);

  auto MULResult = ParentFunc->GetNextAvailableVReg();
  MachineInstruction MUL;
  MUL.SetOpcode(MachineInstruction::MUL);
  MUL.AddOperand(MachineOperand::CreateVirtualRegister(MULResult));
  MUL.AddOperand(MachineOperand::CreateVirtualRegister(DIVResult));
  MUL.AddOperand(ModVReg);
  auto MULIter = ParentBB->InsertAfter(std::move(MUL), &*DIVIter);

  MachineInstruction SUB;
  SUB.SetOpcode(MachineInstruction::SUB);
  SUB.AddOperand(ResVReg);
  SUB.AddOperand(NumVReg);
  SUB.AddOperand(MachineOperand::CreateVirtualRegister(MULResult));
  ParentBB->InsertAfter(std::move(SUB), &*MULIter);

  return true;
}

bool TargetInstructionLegalizer::Expand(MachineInstruction *MI) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::MOD:
    return ExpandMOD(MI);

  default:
    break;
  }

  return false;
}