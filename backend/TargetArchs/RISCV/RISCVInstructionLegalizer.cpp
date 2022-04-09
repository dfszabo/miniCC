#include "RISCVInstructionLegalizer.hpp"
#include "../../MachineBasicBlock.hpp"
#include "../../MachineFunction.hpp"
#include "../../TargetMachine.hpp"
#include "RISCVInstructionDefinitions.hpp"
#include <cassert>

using namespace RISCV;

bool RISCVInstructionLegalizer::Check(MachineInstruction *MI) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::ADD:
    if (MI->GetDef()->GetSize() > TM->GetPointerSize())
      return false;
    break;
  case MachineInstruction::ADDS:
  case MachineInstruction::ADDC:
    return false;
  case MachineInstruction::XOR:
    if (MI->GetDef()->GetSize() > TM->GetPointerSize())
      return false;
    break;
  case MachineInstruction::CMP:
    if (MI->GetOperand(1)->GetSize() > TM->GetPointerSize() ||
        MI->GetOperand(2)->GetSize() > TM->GetPointerSize() ||
        MI->GetRelation() != MachineInstruction::LT)
      return false;
    break;
  case MachineInstruction::LOAD:
    if (MI->GetDef()->GetSize() > TM->GetPointerSize())
      return false;
    break;
  case MachineInstruction::LOAD_IMM:
    if (MI->GetDef()->GetSize() > TM->GetPointerSize())
      return false;
    break;
  case MachineInstruction::STORE:
    if (MI->GetOperands().back().IsImmediate() ||
        MI->GetOperand(1)->GetSize() > TM->GetPointerSize())
      return false;
    break;
  case MachineInstruction::MUL:
    if (MI->GetDef()->GetSize() > TM->GetPointerSize())
      return false;
  case MachineInstruction::DIV:
  case MachineInstruction::DIVU:
  case MachineInstruction::MOD:
  case MachineInstruction::MODU:
    if (MI->GetOperand(2)->IsImmediate())
      return false;
  case MachineInstruction::SUB:
    if (MI->GetOperand(1)->GetSize() > TM->GetPointerSize() ||
        MI->GetOperand(2)->GetSize() > TM->GetPointerSize() ||
        MI->GetOperand(1)->IsImmediate())
      return false;
    break;
  case MachineInstruction::ZEXT:
    if (MI->GetDef()->GetSize() > TM->GetPointerSize())
      return false;
    break;
  case MachineInstruction::TRUNC:
    if (MI->GetOperand(1)->GetSize() > TM->GetPointerSize())
      return false;
    break;
  default:
    break;
  }

  return true;
}

bool RISCVInstructionLegalizer::IsExpandable(const MachineInstruction *MI) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::ADD:
  case MachineInstruction::ADDS:
  case MachineInstruction::ADDC:
  case MachineInstruction::XOR:
  case MachineInstruction::CMP:
  case MachineInstruction::LOAD:
  case MachineInstruction::LOAD_IMM:
  case MachineInstruction::STORE:
  case MachineInstruction::SUB:
  case MachineInstruction::MUL:
  case MachineInstruction::DIV:
  case MachineInstruction::DIVU:
  case MachineInstruction::MOD:
  case MachineInstruction::MODU:
  case MachineInstruction::ZEXT:
  case MachineInstruction::TRUNC:
    return true;

  default:
    break;
  }

  return false;
}

static bool ExpandArithmeticInstWithImm(MachineInstruction *MI, size_t Index) {
  assert(MI->GetOperandsNumber() == 3 && "MI must have exactly 3 operands");
  assert(Index < MI->GetOperandsNumber());
  auto ParentBB = MI->GetParent();

  auto MOV = MachineInstruction(MachineInstruction::LOAD_IMM, nullptr);
  auto DestReg = ParentBB->GetParent()->GetNextAvailableVReg();
  MOV.AddVirtualRegister(DestReg, MI->GetOperand(Index)->GetSize());
  MOV.AddOperand(*MI->GetOperand(Index));

  // replace the immediate operand with the destination of the immediate load
  MI->RemoveOperand(Index);
  MI->InsertOperand(Index, MachineOperand::CreateVirtualRegister(
                               DestReg, MI->GetOperand(0)->GetSize()));

  // insert after modifying MI, otherwise MI would became invalid
  ParentBB->InsertBefore(std::move(MOV), MI);

  return true;
}

bool RISCVInstructionLegalizer::ExpandDIV(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "DIV must have exactly 3 operands");
  return ExpandArithmeticInstWithImm(MI, 2);
}

bool RISCVInstructionLegalizer::ExpandDIVU(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "DIVU must have exactly 3 operands");
  return ExpandArithmeticInstWithImm(MI, 2);
}

bool RISCVInstructionLegalizer::ExpandMOD(MachineInstruction *MI,
                                          bool IsUnsigned) {
  assert(MI->GetOperandsNumber() == 3 && "MOD must have exactly 3 operands");
  return ExpandArithmeticInstWithImm(MI, 2);
}
