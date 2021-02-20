#include "RISCVTargetMachine.hpp"
#include "../../MachineInstruction.hpp"
#include "../../Support.hpp"
#include "../../TargetMachine.hpp"
#include "RISCVInstructionDefinitions.hpp"
#include <cassert>

using namespace RISCV;

bool RISCVTargetMachine::SelectADD(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "ADD must have 3 operands");

  // If last operand is an immediate then select "addi"
  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    assert(IsInt<12>((int64_t)ImmMO->GetImmediate()) &&
           "Immediate must be 12 bit wide");

    // TODO: check if the register operands are valid, like i32 and not f32
    // NOTE: maybe we should not really check here, although then how we know
    // that it is a floating point addition or not?
    MI->SetOpcode(ADDI);
    return true;
  }
  // Try to select "add"
  else {
    MI->SetOpcode(ADD);
    return true;
  }

  return false;
}

bool RISCVTargetMachine::SelectMOD(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "MOD must have 3 operands");

  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    assert(!"Unimplemented");
  } else {
    MI->SetOpcode(REM);
    return true;
  }

  return false;
}

bool RISCVTargetMachine::SelectCMP(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "CMP must have 3 operands");

  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    switch (MI->GetRelation()) {
    case MachineInstruction::LE:
      MI->SetOpcode(SLTI);
      return true;
    default:
      assert(!"Unimplemented");
    }
  } else {
    switch (MI->GetRelation()) {
    case MachineInstruction::LE:
      MI->SetOpcode(SLT);
      return true;
    default:
      assert(!"Unimplemented");
    }
  }

  return false;
}

bool RISCVTargetMachine::SelectLOAD(MachineInstruction *MI) {
  assert((MI->GetOperandsNumber() == 2 || MI->GetOperandsNumber() == 3) &&
         "LOAD must have 2 or 3 operands");

  MI->SetOpcode(LW);
  return true;
}

bool RISCVTargetMachine::SelectSTORE(MachineInstruction *MI) {
  assert((MI->GetOperandsNumber() == 2 || MI->GetOperandsNumber() == 3) &&
         "STORE must have 2 or 3 operands");

  MI->SetOpcode(SW);
  return true;
}

bool RISCVTargetMachine::SelectBRANCH(MachineInstruction *MI) {
  if (MI->IsFallThroughBranch()) {
    MI->SetOpcode(BNEZ);
    return true;
  }

  return false;
}

bool RISCVTargetMachine::SelectJUMP(MachineInstruction *MI) {
  MI->SetOpcode(J);
  return true;
}

bool RISCVTargetMachine::SelectRET(MachineInstruction *MI) {
  MI->SetOpcode(RET);
  return true;
}
