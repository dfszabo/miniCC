#include "AArch64InstructionLegalizer.hpp"
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
  default:
    break;
  }

  return true;
}

bool AArch64InstructionLegalizer::IsExpandable(const MachineInstruction *MI) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::MOD:
  case MachineInstruction::STORE:
    return true;

  default:
    break;
  }

  return false;
}
