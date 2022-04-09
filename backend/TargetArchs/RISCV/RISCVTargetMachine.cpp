#include "RISCVTargetMachine.hpp"
#include "../../MachineBasicBlock.hpp"
#include "../../MachineFunction.hpp"
#include "../../MachineInstruction.hpp"
#include "../../Support.hpp"
#include "../../TargetMachine.hpp"
#include "RISCVInstructionDefinitions.hpp"
#include <cassert>

using namespace RISCV;

// TODO: This should be done in the legalizer
static void ExtendRegSize(MachineOperand *MO, uint8_t BitWidth = 32) {
  if (MO->GetSize() < 32)
    MO->GetTypeRef().SetBitWidth(BitWidth);
}

/// For the given MI the function select its rrr or rri variant based on
/// the MI form. If the immediate does not fit into the instruction @ImmSize
/// width long immediate part, then it will be materialized into a register
bool RISCVTargetMachine::SelectThreeAddressInstruction(MachineInstruction *MI,
                                                       const Opcodes rrr,
                                                       const Opcodes rri,
                                                       const unsigned ImmSize) {
  assert(MI->GetOperandsNumber() == 3 && "MI must have 3 operands");

  ExtendRegSize(MI->GetOperand(0));
  ExtendRegSize(MI->GetOperand(1));

  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    // If the immediate fits
    if (IsInt(ImmMO->GetImmediate(), ImmSize)) {
      MI->SetOpcode(rri);
      return true;
    }

    // If it does not fit, then materialize it first
    auto ParentBB = MI->GetParent();

    auto DestReg = ParentBB->GetParent()->GetNextAvailableVReg();

    MachineInstruction LoadImm;
    LoadImm.SetOpcode(LI);
    LoadImm.AddVirtualRegister(DestReg);
    LoadImm.GetOperand(0)->SetRegClass(GPR32);
    LoadImm.AddOperand(*ImmMO);
    MI = &*ParentBB->InsertBefore(std::move(LoadImm), MI);
    MI++;

    MI->SetOpcode(rrr);
    MI->RemoveOperand(2);
    MI->AddVirtualRegister(DestReg);
    MI->GetOperand(2)->SetRegClass(GPR32);
    ExtendRegSize(MI->GetOperand(2));
    return true;
  } else if (MI->GetOperand(2)->IsRegister() ||
             MI->GetOperand(2)->IsVirtualReg()) {
    ExtendRegSize(MI->GetOperand(2));
    MI->SetOpcode(rrr);
    return true;
  } else
    assert(!"Unreachable");

  return false;
}

bool RISCVTargetMachine::SelectThreeAddressInstruction(MachineInstruction *MI,
                                                       const Opcodes rrr) {
  assert(MI->GetOperandsNumber() == 3 && "MI must have 3 operands");

  ExtendRegSize(MI->GetOperand(0));
  ExtendRegSize(MI->GetOperand(1));

  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    assert(!"Should have handled it in the legalizer");
  } else {
    ExtendRegSize(MI->GetOperand(2));
    MI->SetOpcode(rrr);
    return true;
  }

  return false;
}

bool RISCVTargetMachine::SelectAND(MachineInstruction *MI) {
  if (!SelectThreeAddressInstruction(MI, AND, ANDI))
    assert(!"Cannot select AND");
  return true;
}

bool RISCVTargetMachine::SelectOR(MachineInstruction *MI) {
  if (!SelectThreeAddressInstruction(MI, OR, ORI))
    assert(!"Cannot select OR");
  return true;
}

bool RISCVTargetMachine::SelectXOR(MachineInstruction *MI) {
  if (!SelectThreeAddressInstruction(MI, XOR, XORI))
    assert(!"Cannot select XOR");
  return true;
}

bool RISCVTargetMachine::SelectLSL(MachineInstruction *MI) {
  if (!SelectThreeAddressInstruction(MI, SLL, SLLI))
    assert(!"Cannot select LSL");
  return true;
}

bool RISCVTargetMachine::SelectLSR(MachineInstruction *MI) {
  if (!SelectThreeAddressInstruction(MI, SRL, SRLI))
    assert(!"Cannot select LSR");
  return true;
}

bool RISCVTargetMachine::SelectADD(MachineInstruction *MI) {
  if (!SelectThreeAddressInstruction(MI, ADD, ADDI))
    assert(!"Cannot select ADD");
  return true;
}

bool RISCVTargetMachine::SelectSUB(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "SUB must have 3 operands");

  ExtendRegSize(MI->GetOperand(0));
  ExtendRegSize(MI->GetOperand(1));

  // If last operand is an immediate then select "addi"
  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    if (IsInt(ImmMO->GetImmediate(), 12)) {
      MI->SetOpcode(ADDI);
      MI->GetOperand(2)->SetValue(-ImmMO->GetImmediate());
      return true;
    }

    MI->SetOpcode(SUB);

    // If it does not fit, then materialize it first
    auto ParentBB = MI->GetParent();

    auto DestReg = ParentBB->GetParent()->GetNextAvailableVReg();

    MachineInstruction LoadImm;
    LoadImm.SetOpcode(LI);
    LoadImm.AddVirtualRegister(DestReg);
    LoadImm.AddOperand(*ImmMO);
    MI = &*ParentBB->InsertBefore(std::move(LoadImm), MI);

    MI->SetOpcode(ADDI);
    return true;
  } else {
    MI->SetOpcode(SUB);
    ExtendRegSize(MI->GetOperand(2));
    return true;
  }
}

bool RISCVTargetMachine::SelectMUL(MachineInstruction *MI) {
  return SelectThreeAddressInstruction(MI, MUL);
}

bool RISCVTargetMachine::SelectMULHU(MachineInstruction *MI) {
  return SelectThreeAddressInstruction(MI, MULHU);
}

bool RISCVTargetMachine::SelectDIV(MachineInstruction *MI) {
  return SelectThreeAddressInstruction(MI, DIV);
}

bool RISCVTargetMachine::SelectDIVU(MachineInstruction *MI) {
  return SelectThreeAddressInstruction(MI, DIVU);
}

bool RISCVTargetMachine::SelectMOD(MachineInstruction *MI) {
  return SelectThreeAddressInstruction(MI, REM);
}

bool RISCVTargetMachine::SelectMODU(MachineInstruction *MI) {
  return SelectThreeAddressInstruction(MI, REMU);
}

bool RISCVTargetMachine::SelectCMP(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "CMP must have 3 operands");

  ExtendRegSize(MI->GetOperand(0));
  ExtendRegSize(MI->GetOperand(1));

  if (auto ImmMO = MI->GetOperand(2); ImmMO->IsImmediate()) {
    switch (MI->GetRelation()) {
    case MachineInstruction::LT:
      MI->SetOpcode(SLTI);
      return true;
    default:
      assert(!"Unimplemented");
    }
  } else {
    switch (MI->GetRelation()) {
    case MachineInstruction::LT:
      MI->SetOpcode(SLT);
      ExtendRegSize(MI->GetOperand(2));
      return true;
    default:
      assert(!"Unimplemented");
    }
  }

  return false;
}

bool RISCVTargetMachine::SelectSEXT(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "SEXT must have 2 operands");
  auto Src = MI->GetOperand(1);

  if (Src->GetSize() == 32) {
    MI->SetOpcode(MV);
    ExtendRegSize(MI->GetOperand(1));
    return true;
  }

  assert(Src->GetSize() < 32 && "Cannot extend value over 32 bit");

  auto ParentBB = MI->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Temp = MachineOperand::CreateVirtualRegister(
      ParentBB->GetParent()->GetNextAvailableVReg());
  const unsigned ShiftAmt = 32 - Src->GetSize();
  Temp.SetRegClass(GPR32);

  MI->SetOpcode(SLLI);
  MI->RemoveOperand(0);
  MI->InsertOperand(0, Temp);
  MI->AddImmediate(ShiftAmt);
  ExtendRegSize(MI->GetOperand(1));

  MachineInstruction ArithShiftRight;
  ArithShiftRight.SetOpcode(SRAI);
  ArithShiftRight.AddOperand(Dest);
  ArithShiftRight.AddOperand(Temp);
  ArithShiftRight.AddImmediate(ShiftAmt);
  MI = &*ParentBB->InsertAfter(std::move(ArithShiftRight), MI);

  return true;
}

bool RISCVTargetMachine::SelectZEXT(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "ZEXT must have 2 operands");
  auto Src = MI->GetOperand(1);
  ExtendRegSize(MI->GetOperand(1));

  // Pointless, but does not do any harm
  if (Src->GetSize() == 32) {
    MI->SetOpcode(MV);
    return true;
  }

  assert(Src->GetSize() < 32 && "Cannot extend value over 32 bit");

  if (Src->GetSize() <= 12) {
    MI->SetOpcode(ANDI);
    MI->AddImmediate((1u << Src->GetSize()) - 1);

    return true;
  } else {
    MI->SetOpcode(AND);

    // If it does not fit into the immediate field, then materialize it first
    auto ParentBB = MI->GetParent();

    auto Temp = MachineOperand::CreateVirtualRegister(
        ParentBB->GetParent()->GetNextAvailableVReg());
    Temp.SetRegClass(GPR32);

    MI->AddOperand(Temp);

    MachineInstruction Li;
    Li.SetOpcode(LI);
    Li.AddOperand(Temp);
    Li.AddImmediate((1u << Src->GetSize()) - 1);
    MI = &*ParentBB->InsertBefore(std::move(Li), MI);

    return true;
  }
}

bool RISCVTargetMachine::SelectTRUNC(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "TRUNC must have 2 operands");
  auto Src = MI->GetOperand(1);

  if (MI->GetOperand(0)->GetSize() == 8) {
    // if the operand is an immediate
    if (Src->IsImmediate()) {
      // then calculate the truncated immediate value and issue a MOV
      int64_t ResultImm = Src->GetImmediate() & 0xFFu;
      MI->GetOperand(1)->SetValue(ResultImm);
      MI->SetOpcode(LI);
    } else { // else issue an AND with the mask of 0xFF
      MI->SetOpcode(ANDI);
      MI->AddImmediate(0xFFu);
    }

    ExtendRegSize(MI->GetOperand(0));
    return true;
  } else if (MI->GetOperand(0)->GetSize() == 16) {
    // if the operand is an immediate
    if (Src->IsImmediate()) {
      // then calculate the truncated immediate value and issue a MOV
      int64_t ResultImm = Src->GetImmediate() & 0xFFFFu;
      MI->GetOperand(1)->SetValue(ResultImm);
      MI->SetOpcode(LI);
      ExtendRegSize(MI->GetOperand(0));
      return true;
    }

    // Lower truncation into left and right logical shifts
    //      TRUNC %dst(s16), src
    // into
    //      SLLI  %temp, %src, 16        # src << 16
    //      SRLI  %dst, %temp, 16        # temp >> 16
    auto ParentBB = MI->GetParent();

    auto Dest = *MI->GetOperand(0);
    auto Temp = MachineOperand::CreateVirtualRegister(
        ParentBB->GetParent()->GetNextAvailableVReg());
    const unsigned ShiftAmt = 32 - Dest.GetSize();
    Temp.SetRegClass(GPR32);

    MI->SetOpcode(SLLI);
    MI->RemoveOperand(0);
    MI->InsertOperand(0, Temp);
    MI->AddImmediate(ShiftAmt);
    ExtendRegSize(MI->GetOperand(1));

    MachineInstruction ShR;
    ShR.SetOpcode(SRLI);
    ShR.AddOperand(Dest);
    ShR.AddOperand(Temp);
    ShR.AddImmediate(ShiftAmt);
    MI = &*ParentBB->InsertAfter(std::move(ShR), MI);

    return true;
  } else if (MI->GetOperand(0)->GetSize() == 32) {
    MI->SetOpcode(MV);
    ExtendRegSize(MI->GetOperand(0));
    return true;
  }
  return false;
}

bool RISCVTargetMachine::SelectZEXT_LOAD(MachineInstruction *MI) {
  assert(!"Unimplemented");
}

bool RISCVTargetMachine::SelectLOAD(MachineInstruction *MI) {
  assert((MI->GetOperandsNumber() == 2 || MI->GetOperandsNumber() == 3) &&
         "LOAD must have 2 or 3 operands");

  const auto LoadedValBitSize = MI->GetOperand(0)->GetSize();

  if (LoadedValBitSize <= 8)
    MI->SetOpcode(LB);
  else if (LoadedValBitSize == 16)
    MI->SetOpcode(LH);
  else if (LoadedValBitSize == 32)
    MI->SetOpcode(LW);
  else
    assert(!"Unreachable");

  ExtendRegSize(MI->GetOperand(0));
  return true;
}

bool RISCVTargetMachine::SelectLOAD_IMM(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "LOAD_IMM must have 2 operands");
  assert(MI->GetOperand(1)->IsImmediate() &&
         "Source operand must be and immediate");

  ExtendRegSize(MI->GetOperand(0));
  MI->SetOpcode(LI);

  return true;
}

bool RISCVTargetMachine::SelectMOV(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "MOV must have 2 operands");

  ExtendRegSize(MI->GetOperand(0));

  if (auto ImmMO = MI->GetOperand(1); ImmMO->IsImmediate()) {
    MI->SetOpcode(LI);
    return true;
  } else {
    MI->SetOpcode(MV);
    return true;
  }

  return false;
}

bool RISCVTargetMachine::SelectSTORE(MachineInstruction *MI) {
  assert((MI->GetOperandsNumber() == 2 || MI->GetOperandsNumber() == 3) &&
         "STORE must have 2 or 3 operands");

  auto Src = MI->GetOperand(1);

  if (Src->GetSize() <= 8)
    MI->SetOpcode(SB);
  else if (Src->GetSize() <= 16)
    MI->SetOpcode(SH);
  else if (Src->GetSize() <= 32)
    MI->SetOpcode(SW);
  else
    assert(!"Invalid sized source register for store");

  ExtendRegSize(Src);
  return true;
}

bool RISCVTargetMachine::SelectSTACK_ADDRESS(MachineInstruction *MI) {
  MI->SetOpcode(ADDI);
  return true;
}

bool RISCVTargetMachine::SelectGLOBAL_ADDRESS(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 &&
         "GLOBAL_ADDRESS must have exactly 2 operands");
  auto ParentBB = MI->GetParent();

  auto GlobalVar = *MI->GetOperand(1);
  assert(GlobalVar.IsGlobalSymbol() && "Operand #2 must be a symbol");
  auto GlobalVarHi = "%hi(" + GlobalVar.GetGlobalSymbol() + ")";

  MI->SetOpcode(LUI);
  MI->ReplaceOperand(MachineOperand::CreateGlobalSymbol(GlobalVarHi), 1);

  MachineInstruction addi;
  addi.SetOpcode(ADDI);
  auto DestReg = *MI->GetOperand(0);
  addi.AddOperand(DestReg);
  addi.AddOperand(DestReg);
  auto GlobalVarLo = "%lo(" + GlobalVar.GetGlobalSymbol() + ")";
  addi.AddGlobalSymbol(GlobalVarLo);
  ParentBB->InsertAfter(std::move(addi), MI);

  return true;
}

bool RISCVTargetMachine::SelectBRANCH(MachineInstruction *MI) {
  if (MI->IsFallThroughBranch()) {
    MI->SetOpcode(BNEZ);
    ExtendRegSize(MI->GetOperand(0));
    return true;
  }

  return false;
}

bool RISCVTargetMachine::SelectJUMP(MachineInstruction *MI) {
  MI->SetOpcode(J);
  return true;
}

bool RISCVTargetMachine::SelectCALL(MachineInstruction *MI) {
  MI->SetOpcode(CALL);
  return true;
}

bool RISCVTargetMachine::SelectRET(MachineInstruction *MI) {
  MI->SetOpcode(RET);
  if (MI->GetOperandsNumber() == 1)
    ExtendRegSize(MI->GetOperand(0));
  return true;
}
