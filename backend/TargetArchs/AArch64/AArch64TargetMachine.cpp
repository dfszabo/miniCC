#include "AArch64TargetMachine.hpp"
#include "../../MachineBasicBlock.hpp"
#include "../../Support.hpp"
#include "AArch64InstructionDefinitions.hpp"
#include <cassert>

using namespace AArch64;

bool AArch64TargetMachine::SelectADD(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "ADD must have 3 operands");

  // If last operand is an immediate then select "addi"
  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    // FIXME: Since currently ADD used for adjusting the stack in the prolog,
    // therefore its possible that the immediate is negative. In that case for
    // now we just convert the ADD into a SUB and call select on that.
    if ((int64_t)ImmMO->GetImmediate() < 0) {
      MI->SetOpcode(SUB_rri);
      MI->GetOperand(2)->SetValue(((int64_t)ImmMO->GetImmediate()) * -1);
      return SelectSUB(MI);
    }
    assert(IsUInt<12>((int64_t)ImmMO->GetImmediate()) &&
           "Immediate must be 12 bit wide");

    // TODO: check if the register operands are valid, like i32 and not f32
    // NOTE: maybe we should not really check here, although then how we know
    // that it is a floating point addition or not?
    MI->SetOpcode(ADD_rri);
    return true;
  }
  // Try to select "add"
  else {
    MI->SetOpcode(ADD_rrr);
    return true;
  }

  return false;
}

bool AArch64TargetMachine::SelectSUB(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "SUB must have 3 operands");

  // If last operand is an immediate then select "SUB_rri"
  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    assert(IsUInt<12>((int64_t)ImmMO->GetImmediate()) &&
           "Immediate must be 12 bit wide");

    // TODO: see ADD comment
    MI->SetOpcode(SUB_rri);
    return true;
  }
  // else try to select "SUB_rrr"
  else {
    MI->SetOpcode(SUB_rrr);
    return true;
  }

  return false;
}

bool AArch64TargetMachine::SelectMUL(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "MUL must have 3 operands");

  // If last operand is an immediate then select "MUL_rri"
  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    assert(IsUInt<12>((int64_t)ImmMO->GetImmediate()) &&
           "Immediate must be 12 bit wide");

    // TODO: see ADD comment
    MI->SetOpcode(MUL_rri);
    return true;
  }
  // else try to select "MUL_rrr"
  else {
    MI->SetOpcode(MUL_rrr);
    return true;
  }

  return false;
}

bool AArch64TargetMachine::SelectDIV(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "DIV must have 3 operands");

  // If last operand is an immediate then select "addi"
  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    assert(IsUInt<12>((int64_t)ImmMO->GetImmediate()) &&
           "Immediate must be 12 bit wide");

    // TODO: see ADD comment
    MI->SetOpcode(SDIV_rri);
    return true;
  }
  // else try to select "SDIV_rrr"
  else {
    MI->SetOpcode(SDIV_rrr);
    return true;
  }

  return false;
}

bool AArch64TargetMachine::SelectMOD(MachineInstruction *MI) {
  assert(!"MOD not supported");
  return false;
}

bool AArch64TargetMachine::SelectCMP(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "CMP must have 3 operands");

  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    MI->SetOpcode(CMP_ri);
    // remove the destination hence the implicit condition register is
    // overwritten
    MI->RemoveOperand(0);
    return true;
  } else {
    MI->SetOpcode(CMP_rr);
    // remove the destination hence the implicit condition register is
    // overwritten
    MI->RemoveOperand(0);
    return true;
  }

  return false;
}

bool AArch64TargetMachine::SelectSEXT(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "SEXT must have 2 operands");

  if (MI->GetOperand(1)->GetType().GetBitWidth() == 8) {
    MI->SetOpcode(SXTB);
    return true;
  }

  assert(!"Unimplemented!");
  return false;
}

bool AArch64TargetMachine::SelectTRUNC(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "TRUNC must have 2 operands");

  if (MI->GetOperand(0)->GetType().GetBitWidth() == 8) {
    MI->SetOpcode(AND_rri);
    MI->AddImmediate(0xFFu);
    return true;
  }

  assert(!"Unimplemented!");
  return false;
}

bool AArch64TargetMachine::SelectLOAD_IMM(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 &&
         "LOAD_IMM must have exactly 2 operands");

  assert(MI->GetOperand(1)->IsImmediate() && "Operand #2 must be an immediate");
  assert(IsUInt<16>(MI->GetOperand(1)->GetImmediate()) &&
         "Ivalid immediate value");

  MI->SetOpcode(MOV_rc);
  return true;
}

bool AArch64TargetMachine::SelectMOV(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "MOV must have exactly 2 operands");

  if (MI->GetOperand(1)->IsImmediate()) {
    assert(IsUInt<16>(MI->GetOperand(1)->GetImmediate()) &&
           "Invalid immediate value");
    MI->SetOpcode(MOV_rc);
  } else
    MI->SetOpcode(MOV_rr);

  return true;
}

bool AArch64TargetMachine::SelectLOAD(MachineInstruction *MI) {
  assert((MI->GetOperandsNumber() == 2 || MI->GetOperandsNumber() == 3) &&
         "LOAD must have 2 or 3 operands");

  if (MI->GetOperand(0)->GetType().GetBitWidth() == 8 &&
      !MI->GetOperand(0)->GetType().IsPointer()) {
    MI->SetOpcode(LDRB);
    return true;
  }

  MI->SetOpcode(LDR);
  return true;
}

bool AArch64TargetMachine::SelectSTORE(MachineInstruction *MI) {
  assert((MI->GetOperandsNumber() == 2 || MI->GetOperandsNumber() == 3) &&
         "STORE must have 2 or 3 operands");

  if (MI->GetOperand(MI->GetOperandsNumber() - 1)->GetType().GetBitWidth() == 8) {
    MI->SetOpcode(STRB);
    return true;
  }

  MI->SetOpcode(STR);
  return true;
}

bool AArch64TargetMachine::SelectSTACK_ADDRESS(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 &&
         "STACK_ADDRESS must have 2 operands");

  MI->SetOpcode(ADD_rri);
  return true;
}

bool AArch64TargetMachine::SelectBRANCH(MachineInstruction *MI) {
  // 1) Get the preceding instruction if exists
  // 2) If a compare then use its condition to determine the condition code
  //    for this branch
  // FIXME: not sure if for a branch it is REQUIRED to have a compare before
  //        it or its just optional (likely its optional)
  auto &BBInstructions = MI->GetParent()->GetInstructions();
  MachineInstruction *PrecedingMI = nullptr;

  for (size_t i = 0; i < BBInstructions.size(); i++)
    // find the current instruction index
    if (&BBInstructions[i] == MI && i > 0) {
      PrecedingMI = &BBInstructions[i - 1];
      break;
    }

  if (MI->IsFallThroughBranch()) {
    assert(PrecedingMI && "For now assume a preceding cmp instruction");

    // choose the appropriate conditional branch based on the cmp type
    switch (PrecedingMI->GetRelation()) {
    case MachineInstruction::EQ:
      MI->SetOpcode(BEQ);
      break;
    case MachineInstruction::NE:
      MI->SetOpcode(BNE);
      break;
    case MachineInstruction::LE:
      MI->SetOpcode(BLE);
      break;
    case MachineInstruction::LT:
      MI->SetOpcode(BLT);
      break;
    case MachineInstruction::GE:
      MI->SetOpcode(BGE);
      break;
    case MachineInstruction::GT:
      MI->SetOpcode(BGT);
      break;
    default:
      assert(!"Unimplemented");
    }

    MI->RemoveOperand(0);
    return true;
  }

  return false;
}

bool AArch64TargetMachine::SelectJUMP(MachineInstruction *MI) {
  MI->SetOpcode(B);
  return true;
}

bool AArch64TargetMachine::SelectCALL(MachineInstruction *MI) {
  MI->SetOpcode(BL);
  return true;
}

bool AArch64TargetMachine::SelectRET(MachineInstruction *MI) {
  MI->SetOpcode(RET);
  return true;
}
