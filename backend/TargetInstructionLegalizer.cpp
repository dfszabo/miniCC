#include "TargetInstructionLegalizer.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "TargetMachine.hpp"
#include <cassert>

/// Materialize @MI instruction @Index-th operand - which must be an immediate -
/// into a virtual register by issuing a LOAD_IMM
static bool MaterializeImmOperand(MachineInstruction *MI, const size_t Index) {
  assert(Index < MI->GetOperandsNumber());
  assert(MI->GetOperand(Index)->IsImmediate());
  auto ParentBB = MI->GetParent();

  auto LI = MachineInstruction(MachineInstruction::LOAD_IMM, ParentBB);
  auto DestReg = ParentBB->GetParent()->GetNextAvailableVReg();
  LI.AddVirtualRegister(DestReg, MI->GetOperand(Index)->GetSize());
  LI.AddOperand(*MI->GetOperand(Index));

  // replace the immediate operand with the destination of the immediate load
  MI->RemoveOperand(Index);
  MI->InsertOperand(Index, MachineOperand::CreateVirtualRegister(
                               DestReg, MI->GetOperand(0)->GetSize()));

  // insert after modifying MI, otherwise MI would became invalid
  ParentBB->InsertBefore(std::move(LI), MI);

  return true;
}

bool TargetInstructionLegalizer::IsRelSupported(
    MachineInstruction::CMPRelation Rel) const {
  return UnSupportedRelations.count(Rel) == 0;
}

/// The following
///     ADDS    %dst, %carry, %src1, %src2
///
///  is replaced with
///     ADD     %dst, %src1, %src2
///     CMP.LT  %carry, %dst, %src2
bool TargetInstructionLegalizer::ExpandADDS(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 4 && "ADDS must have exactly 4 operands");
  auto ParentBB = MI->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Carry = *MI->GetOperand(1);
  auto Src1 = *MI->GetOperand(2);
  auto Src2 = *MI->GetOperand(3);

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert(Carry.IsVirtualReg() && "Carry must be a virtual register");
  assert(Src1.IsVirtualReg() && "Source #1 must be a virtual register");
  assert((Src2.IsVirtualReg() || Src2.IsImmediate()) &&
         "Source #2 must be a virtual register or an immediate");

  auto Add = MachineInstruction(MachineInstruction::ADD, ParentBB);
  Add.AddOperand(Dest);
  Add.AddOperand(Src1);
  Add.AddOperand(Src2);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(Add), &*MI);

  auto Cmp = MachineInstruction(MachineInstruction::CMP, ParentBB);
  Cmp.SetAttributes(MachineInstruction::LT);
  Cmp.AddOperand(Carry);
  Cmp.AddOperand(Dest);
  Cmp.AddOperand(Src2);
  ParentBB->InsertAfter(std::move(Cmp), &*InsertTo);

  return true;
}

/// The following
///     ADDC    %dst, %src1, %src2, %carry
///
///  is replaced with
///     ADD     %temp, %src1, %src2
///     ADD     %dst, %temp, %carry
bool TargetInstructionLegalizer::ExpandADDC(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 4 && "ADDC must have exactly 4 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Src1 = *MI->GetOperand(1);
  auto Src2 = *MI->GetOperand(2);
  auto Carry = *MI->GetOperand(3);

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert(Carry.IsVirtualReg() && "Carry must be a virtual register");
  assert(Src1.IsVirtualReg() && "Source #1 must be a virtual register");
  assert((Src2.IsVirtualReg() || Src2.IsImmediate()) &&
         "Source #2 must be a virtual register or an immediate");

  auto Temp =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Add = MachineInstruction(MachineInstruction::ADD, ParentBB);
  Add.AddOperand(Temp);
  Add.AddOperand(Src1);
  Add.AddOperand(Src2);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(Add), &*MI);

  auto Addc = MachineInstruction(MachineInstruction::ADD, ParentBB);
  Addc.AddOperand(Dest);
  Addc.AddOperand(Temp);
  Addc.AddOperand(Carry);
  ParentBB->InsertAfter(std::move(Addc), &*InsertTo);

  return true;
}

/// Expand 64 bit XOR to equivalent calculation using 32 bit additions
///     XOR %dst(s64), %src1(s64), %src2(s64)
///
///  is replaced with
///     SPLIT   %src1_lo(s32), %src1_hi(s32), %src1(s64)
///     SPLIT   %src2_lo(s32), %src2_hi(s32), %src2(s64)
///     XOR     %xor_lo, %src1_lo, %src2_lo
///     XOR     %xor_hi, %src1_hi, %src2_hi
///     MERGE   %dst(s64), %xor_lo, %xor_hi
bool TargetInstructionLegalizer::ExpandXOR(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "XOR must have exactly 3 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Src1 = *MI->GetOperand(1);
  auto Src2 = *MI->GetOperand(2);

  // Assume expansion was requested because the last operand is immediate
  // and the target is not supporting that
  if (Src2.IsImmediate())
    return MaterializeImmOperand(MI, 2);

  assert(Dest.GetSize() == 64 && Src1.GetSize() == Src2.GetSize() &&
         Src1.GetSize() == Dest.GetSize());

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert(Src1.IsVirtualReg() && "Operand #1 must be a virtual register");
  assert((Src2.IsVirtualReg() || Src2.IsImmediate()) &&
         "Operand #2 must be a virtual register or an immediate");

  auto Src1Lo =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Src1Hi =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Src2Lo =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Src2Hi =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Split1 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  Split1.AddOperand(Src1Lo);
  Split1.AddOperand(Src1Hi);
  Split1.AddOperand(Src1);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(Split1), MI);

  auto Split2 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  Split2.AddOperand(Src2Lo);
  Split2.AddOperand(Src2Hi);
  Split2.AddOperand(Src2);
  InsertTo = ParentBB->InsertAfter(std::move(Split2), &*InsertTo);

  auto Xor1Dst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Xor1 = MachineInstruction(MachineInstruction::XOR, ParentBB);
  Xor1.AddOperand(Xor1Dst);
  Xor1.AddOperand(Src1Lo);
  Xor1.AddOperand(Src2Lo);
  InsertTo = ParentBB->InsertAfter(std::move(Xor1), &*InsertTo);

  auto Xor2Dst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Xor2 = MachineInstruction(MachineInstruction::XOR, ParentBB);
  Xor2.AddOperand(Xor2Dst);
  Xor2.AddOperand(Src1Hi);
  Xor2.AddOperand(Src2Hi);
  InsertTo = ParentBB->InsertAfter(std::move(Xor2), &*InsertTo);

  auto Merge = MachineInstruction(MachineInstruction::MERGE, ParentBB);
  Merge.AddOperand(Dest);
  Merge.AddOperand(Xor1Dst);
  Merge.AddOperand(Xor2Dst);
  ParentBB->InsertAfter(std::move(Merge), &*InsertTo);

  return true;
}

/// Expand 64 bit MUL to equivalent calculation using 32 bit muls and adds
///     MUL %dst(s64), %src1(s64), %src2(s64)
///
///  is replaced with
///     SPLIT   %src1_lo(s32), %src1_hi(s32), %src1(s64)
///     SPLIT   %src2_lo(s32), %src2_hi(s32), %src2(s64)
///     MUL     %mul1, %src1_hi, %src2_lo
///     MUL     %mul2, %src2_hi, %src1_lo
///     ADD     %add1, %mul1, %mul2
///     MULHU   %mulhu, %src1_lo, %src2_lo
///     MUL     %dst_lo, %src1_lo, %src2_lo
///     ADD     %dst_hi, %add1, %mulhu
///     MERGE   %dst(s64), %dst_lo(s32), %dst_hi(s32)
bool TargetInstructionLegalizer::ExpandMUL(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "MUL must have exactly 3 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Src1 = *MI->GetOperand(1);
  auto Src2 = *MI->GetOperand(2);

  // If the expansion was requested because the last operand is immediate
  // and the target is not supporting that
  if (Src2.IsImmediate() && Dest.GetSize() <= TM->GetPointerSize())
    return MaterializeImmOperand(MI, 2);

  if (Src2.IsImmediate())
    return MaterializeImmOperand(MI, 2);

  assert(Dest.GetSize() == 64 && Src1.GetSize() == Src2.GetSize() &&
         Src1.GetSize() == Dest.GetSize());

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert(Src1.IsVirtualReg() && "Operand #1 must be a virtual register");
  assert(Src2.IsVirtualReg() && "Operand #2 must be a virtual register");

  auto Src1Lo =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Src1Hi =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Src2Lo =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Src2Hi =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto SplitSrc1 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  SplitSrc1.AddOperand(Src1Lo);
  SplitSrc1.AddOperand(Src1Hi);
  SplitSrc1.AddOperand(Src1);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(SplitSrc1), MI);

  auto SplitSrc2 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  SplitSrc2.AddOperand(Src2Lo);
  SplitSrc2.AddOperand(Src2Hi);
  SplitSrc2.AddOperand(Src2);
  InsertTo = ParentBB->InsertAfter(std::move(SplitSrc2), &*InsertTo);

  auto Mul1Dst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Mul1 = MachineInstruction(MachineInstruction::MUL, ParentBB);
  Mul1.AddOperand(Mul1Dst);
  Mul1.AddOperand(Src1Hi);
  Mul1.AddOperand(Src2Lo);
  InsertTo = ParentBB->InsertAfter(std::move(Mul1), &*InsertTo);

  auto Mul2Dst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Mul2 = MachineInstruction(MachineInstruction::MUL, ParentBB);
  Mul2.AddOperand(Mul2Dst);
  Mul2.AddOperand(Src2Hi);
  Mul2.AddOperand(Src1Lo);
  InsertTo = ParentBB->InsertAfter(std::move(Mul2), &*InsertTo);

  auto Add1Dst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Add1 = MachineInstruction(MachineInstruction::ADD, ParentBB);
  Add1.AddOperand(Add1Dst);
  Add1.AddOperand(Mul1Dst);
  Add1.AddOperand(Mul2Dst);
  InsertTo = ParentBB->InsertAfter(std::move(Add1), &*InsertTo);

  auto MulhuDst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Mulhu = MachineInstruction(MachineInstruction::MULHU, ParentBB);
  Mulhu.AddOperand(MulhuDst);
  Mulhu.AddOperand(Src1Lo);
  Mulhu.AddOperand(Src2Lo);
  InsertTo = ParentBB->InsertAfter(std::move(Mulhu), &*InsertTo);

  auto DstLo =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Mul3 = MachineInstruction(MachineInstruction::MUL, ParentBB);
  Mul3.AddOperand(DstLo);
  Mul3.AddOperand(Src1Lo);
  Mul3.AddOperand(Src2Lo);
  InsertTo = ParentBB->InsertAfter(std::move(Mul3), &*InsertTo);

  auto DstHi =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Add2 = MachineInstruction(MachineInstruction::ADD, ParentBB);
  Add2.AddOperand(DstHi);
  Add2.AddOperand(Add1Dst);
  Add2.AddOperand(MulhuDst);
  InsertTo = ParentBB->InsertAfter(std::move(Add2), &*InsertTo);

  auto Merge = MachineInstruction(MachineInstruction::MERGE, ParentBB);
  Merge.AddOperand(Dest);
  Merge.AddOperand(DstLo);
  Merge.AddOperand(DstHi);
  ParentBB->InsertAfter(std::move(Merge), &*InsertTo);

  return true;
}

/// Expand 64 bit ADD to equivalent calculation using 32 bit additions
///     ADD     %dst(s64), %src1(s64), %src2(s64)
///
///  is replaced with
///     SPLIT   %src1_lo(s32), %src1_hi(s32), %src1(s64)
///     SPLIT   %src2_lo(s32), %src2_hi(s32), %src2(s64)
///     ADDS    %adds, %carry, %src1_lo, %src2_lo
///     ADDC    %addc, %src1_hi, %src2_hi, %carry
///     MERGE   %dst(s64), %adds(s32), %addc(s32)
bool TargetInstructionLegalizer::ExpandADD(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "ADD must have exactly 3 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Src1 = *MI->GetOperand(1);
  auto Src2 = *MI->GetOperand(2);

  // Better to materialize the constant in this case
  // TODO: If the immediate fits into s32 then the generated code could
  // certainly be smaller. Its pointless to treat its as s64. Improve it.
  if (Src2.IsImmediate()) {
    // Make sure that the operand is also has the same size as
    // the destination
    // TODO: This should be already the case
    MI->GetOperand(2)->SetSize(Dest.GetSize());
    Src2.SetSize(Dest.GetSize());
    return MaterializeImmOperand(MI, 2);
  }

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert(Src1.IsVirtualReg() && "Operand #1 must be a virtual register");
  assert(Src2.IsVirtualReg() && "Operand #2 must be a virtual register");

  auto SplitSrc1Lo =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto SplitSrc1Hi =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto SplitSrc2Lo =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto SplitSrc2Hi =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto SplitSrc1 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  SplitSrc1.AddOperand(SplitSrc1Lo);
  SplitSrc1.AddOperand(SplitSrc1Hi);
  SplitSrc1.AddOperand(Src1);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(SplitSrc1), MI);

  auto SplitSrc2 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  SplitSrc2.AddOperand(SplitSrc2Lo);
  SplitSrc2.AddOperand(SplitSrc2Hi);
  SplitSrc2.AddOperand(Src2);
  InsertTo = ParentBB->InsertAfter(std::move(SplitSrc2), &*InsertTo);

  auto AddLoDst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Carry = MachineOperand::CreateVirtualRegister(
      ParentFunc->GetNextAvailableVReg(), 1);

  auto Adds = MachineInstruction(MachineInstruction::ADDS, ParentBB);
  Adds.AddOperand(AddLoDst);
  Adds.AddOperand(Carry);
  Adds.AddOperand(SplitSrc1Lo);
  Adds.AddOperand(SplitSrc2Lo);
  InsertTo = ParentBB->InsertAfter(std::move(Adds), &*InsertTo);

  auto AddHiDst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Addc = MachineInstruction(MachineInstruction::ADDC, ParentBB);
  Addc.AddOperand(AddHiDst);
  Addc.AddOperand(SplitSrc1Hi);
  Addc.AddOperand(SplitSrc2Hi);
  Addc.AddOperand(Carry);
  InsertTo = ParentBB->InsertAfter(std::move(Addc), &*InsertTo);

  auto Merge = MachineInstruction(MachineInstruction::MERGE, ParentBB);
  Merge.AddOperand(Dest);
  Merge.AddOperand(AddLoDst);
  Merge.AddOperand(AddHiDst);
  ParentBB->InsertAfter(std::move(Merge), &*InsertTo);

  return true;
}

bool TargetInstructionLegalizer::ExpandCMP(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "CMP must have exactly 3 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  const bool IsEQ = MI->GetRelation() == MachineInstruction::EQ;
  const bool IsNE = MI->GetRelation() == MachineInstruction::NE;
  [[maybe_unused]] const bool IsLT =
      MI->GetRelation() == MachineInstruction::LT;
  const bool IsGT = MI->GetRelation() == MachineInstruction::GT;
  const bool IsLE = MI->GetRelation() == MachineInstruction::LE;
  const bool IsGE = MI->GetRelation() == MachineInstruction::GE;

  const bool HasEQ = IsRelSupported(MachineInstruction::EQ);
  const bool HasNE = IsRelSupported(MachineInstruction::NE);
  const bool HasLT = IsRelSupported(MachineInstruction::LT);
  const bool HasGT = IsRelSupported(MachineInstruction::GT);
  const bool HasLE = IsRelSupported(MachineInstruction::LE);
  const bool HasGE = IsRelSupported(MachineInstruction::GE);

  // case when the sources are s64 sized, but it is not supported
  if (MI->GetOperand(1)->GetSize() == 64 ||
      MI->GetOperand(2)->GetSize() == 64) {
    auto ParentBB = MI->GetParent();
    auto ParentFunc = ParentBB->GetParent();

    auto Dest = *MI->GetOperand(0);
    auto Src1 = *MI->GetOperand(1);
    auto Src2 = *MI->GetOperand(2);

    // Better to materialize the constant in this case
    if (Src2.IsImmediate())
      return MaterializeImmOperand(MI, 2);

    assert(Dest.IsVirtualReg() && "Result must be a virtual register");
    assert(Src1.IsVirtualReg() && "Operand #1 must be a virtual register");
    assert(Src2.IsVirtualReg() && "Operand #2 must be a virtual register");

    // EQ and NE case
    //    CMP.[EQ|NE] %res, %src1(s64), %src2(s64)
    //
    // is replaced with
    //    SPLIT       %src1_lo, %src1_hi, %src1
    //    SPLIT       %src2_lo, %src2_hi, %src2
    //    XOR         %xor_lo, %src1_lo, %src2_lo
    //    XOR         %xor_hi, %src1_hi, %src2_hi
    //    OR          %or, %xor_lo, %xor_hi
    //    CMP.[EQ|NE] %res, %or, 0
    if (IsEQ || IsNE) {
      auto Src1Lo = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());
      auto Src1Hi = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());
      auto Src2Lo = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());
      auto Src2Hi = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto Split1 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
      Split1.AddOperand(Src1Lo);
      Split1.AddOperand(Src1Hi);
      Split1.AddOperand(Src1);
      auto InsertTo = ParentBB->ReplaceInstr(std::move(Split1), MI);

      auto Split2 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
      Split2.AddOperand(Src2Lo);
      Split2.AddOperand(Src2Hi);
      Split2.AddOperand(Src2);
      InsertTo = ParentBB->InsertAfter(std::move(Split2), &*InsertTo);

      auto XorLoDst = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto XorLo = MachineInstruction(MachineInstruction::XOR, ParentBB);
      XorLo.AddOperand(XorLoDst);
      XorLo.AddOperand(Src1Lo);
      XorLo.AddOperand(Src2Lo);
      InsertTo = ParentBB->InsertAfter(std::move(XorLo), &*InsertTo);

      auto XorHiDst = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto XorHi = MachineInstruction(MachineInstruction::XOR, ParentBB);
      XorHi.AddOperand(XorHiDst);
      XorHi.AddOperand(Src1Hi);
      XorHi.AddOperand(Src2Hi);
      InsertTo = ParentBB->InsertAfter(std::move(XorHi), &*InsertTo);

      auto OrDst = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto Or = MachineInstruction(MachineInstruction::OR, ParentBB);
      Or.AddOperand(OrDst);
      Or.AddOperand(XorLoDst);
      Or.AddOperand(XorHiDst);
      InsertTo = ParentBB->InsertAfter(std::move(Or), &*InsertTo);

      auto Cmp = MachineInstruction(MachineInstruction::CMP, ParentBB);
      Cmp.SetAttributes(IsEQ ? MachineInstruction::EQ : MachineInstruction::NE);
      Cmp.AddOperand(Dest);
      Cmp.AddOperand(OrDst);
      Cmp.AddImmediate(0);
      ParentBB->InsertAfter(std::move(Cmp), &*InsertTo);

      return true;
    }

    // LT, LE, GT, GE
    else {
      // This could be done with 5 instructions, but have to introduce
      // new basic blocks, so instead branchless programming principles used to
      // solve it without branches, but this results in a sequence of 7
      // instructions. And also with branches some unnecessary calculation can
      // be skipped.
      // TODO: Chose this branchless way, because not sure if currently
      // inserting a BB would not screw up something (reallocation of the vector
      // of basic block may cause problems or simply the way the basic blocks
      // iterated in the passes etc). Investigate this and improve the
      // implementation

      // EQ and NE case
      //    CMP.<REL> %res, %src1(s64), %src2(s64)
      //
      // is replaced with
      //    SPLIT       %src1_lo, %src1_hi, %src1
      //    SPLIT       %src2_lo, %src2_hi, %src2
      //    CMP.EQ      %hi_eq, %src1_hi, %src2_hi
      //    CMP.NE      %hi_ne, %src1_hi, %src2_hi
      //    CMP.<REL>   %cmp_lo, %src1_lo, %src2_lo
      //    CMP.<REL>   %cmp_hi, %src1_hi, %src2_hi
      //    MUL         %mul_eq, %hi_eq, %cmp_lo
      //    MUL         %mul_ne, %hi_ne, %cmp_hi
      //    ADD         %add, %mul_eq, %mul_ne
      auto Src1Lo = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());
      auto Src1Hi = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());
      auto Src2Lo = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());
      auto Src2Hi = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      const auto Relation = MI->GetRelation();

      auto Split1 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
      Split1.AddOperand(Src1Lo);
      Split1.AddOperand(Src1Hi);
      Split1.AddOperand(Src1);
      auto InsertTo = ParentBB->ReplaceInstr(std::move(Split1), MI);

      auto Split2 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
      Split2.AddOperand(Src2Lo);
      Split2.AddOperand(Src2Hi);
      Split2.AddOperand(Src2);
      InsertTo = ParentBB->InsertAfter(std::move(Split2), &*InsertTo);

      auto CmpHiEqDst = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto CmpHiEq = MachineInstruction(MachineInstruction::CMP, ParentBB);
      CmpHiEq.SetAttributes(MachineInstruction::EQ);
      CmpHiEq.AddOperand(CmpHiEqDst);
      CmpHiEq.AddOperand(Src1Hi);
      CmpHiEq.AddOperand(Src2Hi);
      InsertTo = ParentBB->InsertAfter(std::move(CmpHiEq), &*InsertTo);

      auto CmpHiNeDst = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto CmpHiNe = MachineInstruction(MachineInstruction::CMP, ParentBB);
      CmpHiNe.SetAttributes(MachineInstruction::NE);
      CmpHiNe.AddOperand(CmpHiNeDst);
      CmpHiNe.AddOperand(Src1Hi);
      CmpHiNe.AddOperand(Src2Hi);
      InsertTo = ParentBB->InsertAfter(std::move(CmpHiNe), &*InsertTo);

      auto CmpLoDst = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto CmpLo = MachineInstruction(MachineInstruction::CMP, ParentBB);
      CmpLo.SetAttributes(Relation);
      CmpLo.AddOperand(CmpLoDst);
      CmpLo.AddOperand(Src1Lo);
      CmpLo.AddOperand(Src2Lo);
      InsertTo = ParentBB->InsertAfter(std::move(CmpLo), &*InsertTo);

      auto CmpHiDst = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto CmpHi = MachineInstruction(MachineInstruction::CMP, ParentBB);
      CmpHi.SetAttributes(Relation);
      CmpHi.AddOperand(CmpHiDst);
      CmpHi.AddOperand(Src1Hi);
      CmpHi.AddOperand(Src2Hi);
      InsertTo = ParentBB->InsertAfter(std::move(CmpHi), &*InsertTo);

      auto MulHiEqDst = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto MulHiEq = MachineInstruction(MachineInstruction::MUL, ParentBB);
      MulHiEq.AddOperand(MulHiEqDst);
      MulHiEq.AddOperand(CmpHiEqDst);
      MulHiEq.AddOperand(CmpLoDst);
      InsertTo = ParentBB->InsertAfter(std::move(MulHiEq), &*InsertTo);

      auto MulHiNeDst = MachineOperand::CreateVirtualRegister(
          ParentFunc->GetNextAvailableVReg());

      auto MulHiNe = MachineInstruction(MachineInstruction::MUL, ParentBB);
      MulHiNe.AddOperand(MulHiNeDst);
      MulHiNe.AddOperand(CmpHiNeDst);
      MulHiNe.AddOperand(CmpHiDst);
      InsertTo = ParentBB->InsertAfter(std::move(MulHiNe), &*InsertTo);

      auto Add = MachineInstruction(MachineInstruction::ADD, ParentBB);
      Add.AddOperand(Dest);
      Add.AddOperand(MulHiEqDst);
      Add.AddOperand(MulHiNeDst);
      ParentBB->InsertAfter(std::move(Add), &*InsertTo);

      return true;
    }
  }

  // EQ and NE case
  //    CMP.EQ %res, %src1, %src2
  //
  // is replaced with
  //    XOR     %dst, $scr1, $src2
  //    CMP.LT  %res, %dst, 1
  //
  // and
  //    CMP.NE %res, %src1, %src2
  //
  // is replaced with
  //    XOR     %dst, $scr1, $src2
  //    CMP.LT  %res, 0, %dst
  //
  // given that LT is supported
  if (((IsNE && !HasNE) || (IsEQ && !HasEQ)) && HasLT) {
    // change the relation of the compare to less than
    MI->SetAttributes(MachineInstruction::LT);

    auto XorDst = ParentFunc->GetNextAvailableVReg();
    auto XorDstMO = MachineOperand::CreateVirtualRegister(
        XorDst, MI->GetOperand(1)->GetSize());

    MachineInstruction Xor;
    Xor.SetOpcode(MachineInstruction::XOR);
    Xor.AddOperand(XorDstMO);
    Xor.AddOperand(*MI->GetOperand(1));
    Xor.AddOperand(*MI->GetOperand(2));
    MI = &*ParentBB->InsertBefore(std::move(Xor), MI);
    MI++;

    // Replace the immediate operand with the result register
    MI->RemoveOperand(2);
    MI->RemoveOperand(1);
    if (IsEQ) {
      MI->AddOperand(XorDstMO);
      MI->AddImmediate(1);
    } else {
      // If the target has zero register, then use that
      if (auto ZeroReg = TM->GetRegInfo()->GetZeroRegister(XorDstMO.GetSize());
          ZeroReg != ~0u)
        MI->AddRegister(ZeroReg);
      else
        // TODO: load 0 into a register first
        MI->AddImmediate(0);

      MI->AddOperand(XorDstMO);
    }
    return true;
  }
  // change CMP.GT %dst, %src1, %src2 to CMP.LT %dst, %src2, %src1
  else if (IsGT && !HasGT && HasLT) {
    MI->SetAttributes(MachineInstruction::LT);

    auto src2 = *MI->GetOperand(2);

    // If the 2nd source operand is an immediate then load it
    // in first to a register since it will become the first
    // source operand which cannot be an immediate
    if (src2.IsImmediate()) {
      // If the immediate is zero and the target has zero register,
      // then use that instead
      if (src2.GetImmediate() == 0 &&
          TM->GetRegInfo()->GetZeroRegister(src2.GetSize()) != ~0u) {
        auto ZeroReg = TM->GetRegInfo()->GetZeroRegister(src2.GetSize());
        src2 = MachineOperand::CreateRegister(ZeroReg, src2.GetSize());
      }
      // Else the immediate is non-zero or the target does not have
      // zero register, so issue a LOAD_IMM first for src2
      else {
        auto dst = MachineOperand::CreateVirtualRegister(
            ParentFunc->GetNextAvailableVReg());

        MachineInstruction LI;
        LI.SetOpcode(MachineInstruction::LOAD_IMM);
        LI.AddOperand(dst);
        LI.AddOperand(src2);
        MI = &*ParentBB->InsertBefore(std::move(LI), MI);
        src2 = dst;
        MI++;
      }
    }

    MI->RemoveOperand(2);
    MI->InsertOperand(1, src2);
    return true;
  }
  // GE
  //    CMP.GE %res, %src1, %src2
  //
  // is replaced with
  //    CMP.LT  %dst, %src1, %src2
  //    XOR     %res, $dst, 1
  //
  // The XOR is doing negation, so turning x >= y to !(x < y)
  else if (IsGE && !HasGE && HasLT) {
    MI->SetAttributes(MachineInstruction::LT);

    auto dst = MachineOperand::CreateVirtualRegister(
        ParentFunc->GetNextAvailableVReg(), MI->GetOperand(0)->GetSize());

    auto res = *MI->GetOperand(0);

    // replace CMP def operand to the new virtual register
    MI->RemoveOperand(0);
    MI->InsertOperand(0, dst);

    // building XOR %res, $dst, 1, which is basically res = !dst
    MachineInstruction Xor;
    Xor.SetOpcode(MachineInstruction::XOR);
    Xor.AddOperand(res);
    Xor.AddOperand(dst);
    Xor.AddImmediate(1);
    ParentBB->InsertAfter(std::move(Xor), MI);

    return true;
  }
  // LE case
  //    CMP.LE %res, %src1, %src2
  //
  // is replaced with
  //    CMP.GE  %res, %src2, %src1
  // Just turning it into GE and that could be expanded as it can be seen above
  else if (IsLE && !HasLE && HasLT) {
    MI->SetAttributes(MachineInstruction::GE);

    auto src2 = *MI->GetOperand(2);

    // If the 2nd source operand is an immediate then load it
    // in first to a register since it will become the first
    // source operand which cannot be an immediate
    if (src2.IsImmediate()) {
      // If the immediate is zero and the target has zero register,
      // then use that instead
      if (src2.GetImmediate() == 0 &&
          TM->GetRegInfo()->GetZeroRegister(src2.GetSize()) != ~0u) {
        auto ZeroReg = TM->GetRegInfo()->GetZeroRegister(src2.GetSize());
        src2 = MachineOperand::CreateRegister(ZeroReg, src2.GetSize());
      }
      // Else the immediate is non-zero or the target does not have
      // zero register, so issue a LOAD_IMM first for src2
      else {
        auto dst = MachineOperand::CreateVirtualRegister(
            ParentFunc->GetNextAvailableVReg());

        MachineInstruction LI;
        LI.SetOpcode(MachineInstruction::LOAD_IMM);
        LI.AddOperand(dst);
        LI.AddOperand(src2);
        MI = &*ParentBB->InsertBefore(std::move(LI), MI);
        src2 = dst;
        MI++;
      }
    }

    MI->RemoveOperand(2);
    MI->InsertOperand(1, src2);
    return true;
  }

  return false;
}

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
  auto DIV = MachineInstruction(IsUnsigned ? MachineInstruction::DIVU
                                           : MachineInstruction::DIV,
                                ParentBB);
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

/// If the target does not support s64, then
/// the following
///     LOAD %dest(s64), [address]
///
/// is replaced with
///     LOAD %lo32(s32), [address]
///     LOAD %hi32(s32), [address+4]
///     MERGE %dest(s64), %lo32(s32), %hi32(s32)
///
/// where immediate is a constant number
bool TargetInstructionLegalizer::ExpandLOAD(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "LOAD must have exactly 2 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Address = *MI->GetOperand(1);

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert((Address.IsMemory() || Address.IsStackAccess()) &&
         "Result must be a memory access");

  auto Lo32 =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Hi32 =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  MI->RemoveOperand(0);
  MI->InsertOperand(0, Lo32);

  auto LoadHI = MachineInstruction(MachineInstruction::LOAD, ParentBB);
  LoadHI.AddOperand(Hi32);
  LoadHI.AddOperand(Address);
  LoadHI.GetOperand(1)->SetOffset(Address.GetOffset() + 4);
  auto InsertTo = ParentBB->InsertAfter(std::move(LoadHI), MI);

  auto Merge = MachineInstruction(MachineInstruction::MERGE, ParentBB);
  Merge.AddOperand(Dest);
  Merge.AddOperand(Lo32);
  Merge.AddOperand(Hi32);
  ParentBB->InsertAfter(std::move(Merge), &*InsertTo);

  return true;
}

/// Expand 64 bit LOAD_IMM to equivalent calculation using 32 bit instructions
///     LOAD_IMM   %dst(s64), const
///
///  is replaced with
///     LOAD_IMM   %lo(s32), const & 0xffffffff
///     LOAD_IMM   %hi(s32), (const >> 32) & 0xffffffff
///     MERGE      %dst(s64), %lo(s32), %hi(s32)
bool TargetInstructionLegalizer::ExpandLOAD_IMM(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 &&
         "LOAD_IMM must have exactly 2 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Src = *MI->GetOperand(1);

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert(Src.IsImmediate() && "Source must be an immediate");

  auto Lo32 =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Hi32 =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  MachineInstruction LI1;
  LI1.SetOpcode(MachineInstruction::LOAD_IMM);
  LI1.AddOperand(Lo32);
  LI1.AddImmediate(Src.GetImmediate() & 0xffffffff);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(LI1), MI);

  MachineInstruction LI2;
  LI2.SetOpcode(MachineInstruction::LOAD_IMM);
  LI2.AddOperand(Hi32);
  LI2.AddImmediate((Src.GetImmediate() >> 32u) & 0xffffffff);
  InsertTo = ParentBB->InsertAfter(std::move(LI2), &*InsertTo);

  auto Merge = MachineInstruction(MachineInstruction::MERGE, ParentBB);
  Merge.AddOperand(Dest);
  Merge.AddOperand(Lo32);
  Merge.AddOperand(Hi32);
  ParentBB->InsertAfter(std::move(Merge), &*InsertTo);

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

  auto Address = *MI->GetOperand(0);
  auto Src = *MI->GetOperand(1);

  if (Src.IsImmediate()) {
    // Create the result register where the immediate will be loaded
    auto LOAD_IMMResult = ParentFunc->GetNextAvailableVReg();
    auto LOAD_IMMResultVReg =
        MachineOperand::CreateVirtualRegister(LOAD_IMMResult, Src.GetSize());

    // Replace the immediate operand with the result register
    MI->RemoveOperand(1);
    MI->AddOperand(LOAD_IMMResultVReg);

    MachineInstruction LI;
    LI.SetOpcode(MachineInstruction::LOAD_IMM);
    LI.AddOperand(LOAD_IMMResultVReg);
    LI.AddOperand(Src);
    ParentBB->InsertBefore(std::move(LI), MI);

    return true;
  }

  /// If the target does not support s64, then
  /// the following
  ///     STORE [address], %src(s64)
  ///
  /// is replaced with
  ///     SPLIT %lo32(s32), %hi32(s32), %src(s64)
  ///     STORE [address], %lo32(s32)
  ///     STORE [address+4], %hi32(s32)
  ///     MERGE %dest(s64), %lo32(s32), %hi32(s32)
  auto Lo32 =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Hi32 =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Split = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  Split.AddOperand(Lo32);
  Split.AddOperand(Hi32);
  Split.AddOperand(Src);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(Split), MI);

  auto StoreLo = MachineInstruction(MachineInstruction::STORE, ParentBB);
  StoreLo.AddOperand(Address);
  StoreLo.AddOperand(Lo32);
  StoreLo.GetOperand(0)->SetOffset(Address.GetOffset());
  InsertTo = ParentBB->InsertAfter(std::move(StoreLo), &*InsertTo);

  auto StoreHi = MachineInstruction(MachineInstruction::STORE, ParentBB);
  StoreHi.AddOperand(Address);
  StoreHi.AddOperand(Hi32);
  StoreHi.GetOperand(0)->SetOffset(Address.GetOffset() + 4);
  ParentBB->InsertAfter(std::move(StoreHi), &*InsertTo);

  return true;
}

/// Expand 64 bit SUB to equivalent calculation using 32 bit substractions
/// The following
///     SUB     %dst(s64), %src1(s64), %src1(s64)
///
/// is replaced with
///     SPLIT   %src1_lo(s32), %src1_hi(s32), %src1(s64)
///     SPLIT   %src2_lo(s32), %src2_hi(s32), %src2(s64)
///     CMP.LT  %cmp_lo, %src1_lo, %src2_lo
///     SUB     %sub1, %src1_hi, %src2_hi
///     SUB     %sub_hi, %sub1, %cmp_lo
///     SUB     %sub_lo, %src1_lo, %src2_lo
///     MERGE   %dest(s64), %sub_lo(s32), %sub_hi(s32)
bool TargetInstructionLegalizer::ExpandSUB(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 3 && "SUB must have exactly 3 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Src1 = *MI->GetOperand(1);
  auto Src2 = *MI->GetOperand(2);

  if (Src1.IsImmediate())
    return MaterializeImmOperand(MI, 1);

  // Better to materialize the constant in this case
  // TODO: If the immediate fits into s32 then the generated code could
  // certainly be smaller. Its pointless to treat its as s64. Improve it.
  if (Src2.IsImmediate()) {
    // Make sure that the operand is also has the same size as
    // the destination
    // TODO: This should be already the case
    MI->GetOperand(2)->SetSize(Dest.GetSize());
    Src2.SetSize(Dest.GetSize());
    return MaterializeImmOperand(MI, 2);
  }

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert(Src1.IsVirtualReg() && "Operand #1 must be a virtual register");
  assert(Src2.IsVirtualReg() && "Operand #2 must be a virtual register");

  auto Src1Lo =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Src1Hi =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Src2Lo =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Src2Hi =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto SplitSrc1 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  SplitSrc1.AddOperand(Src1Lo);
  SplitSrc1.AddOperand(Src1Hi);
  SplitSrc1.AddOperand(Src1);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(SplitSrc1), MI);

  auto SplitSrc2 = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  SplitSrc2.AddOperand(Src2Lo);
  SplitSrc2.AddOperand(Src2Hi);
  SplitSrc2.AddOperand(Src2);
  InsertTo = ParentBB->InsertAfter(std::move(SplitSrc2), &*InsertTo);

  auto CmpLoDst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto CmpLo = MachineInstruction(MachineInstruction::CMP, ParentBB);
  CmpLo.SetAttributes(MachineInstruction::LT);
  CmpLo.AddOperand(CmpLoDst);
  CmpLo.AddOperand(Src1Lo);
  CmpLo.AddOperand(Src2Lo);
  InsertTo = ParentBB->InsertAfter(std::move(CmpLo), &*InsertTo);

  auto Sub1Dst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Sub1 = MachineInstruction(MachineInstruction::SUB, ParentBB);
  Sub1.AddOperand(Sub1Dst);
  Sub1.AddOperand(Src1Hi);
  Sub1.AddOperand(Src2Hi);
  InsertTo = ParentBB->InsertAfter(std::move(Sub1), &*InsertTo);

  auto SubLoDst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto SubLo = MachineInstruction(MachineInstruction::SUB, ParentBB);
  SubLo.AddOperand(SubLoDst);
  SubLo.AddOperand(Sub1Dst);
  SubLo.AddOperand(CmpLoDst);
  InsertTo = ParentBB->InsertAfter(std::move(SubLo), &*InsertTo);

  auto SubHiDst =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto SubHi = MachineInstruction(MachineInstruction::SUB, ParentBB);
  SubHi.AddOperand(SubHiDst);
  SubHi.AddOperand(Src1Hi);
  SubHi.AddOperand(Src2Hi);
  InsertTo = ParentBB->InsertAfter(std::move(SubHi), &*InsertTo);

  auto Merge = MachineInstruction(MachineInstruction::MERGE, ParentBB);
  Merge.AddOperand(Dest);
  Merge.AddOperand(SubLoDst);
  Merge.AddOperand(SubHiDst);
  ParentBB->InsertAfter(std::move(Merge), &*InsertTo);

  return true;
}

/// Expand 64 bit ZEXT to equivalent calculation using 32 bit instructions
///     ZEXT %dst(s64), %src(s32)
///  is replaced with
///     LOAD_IMM   %zero, 0
///     MERGE      %dst(s64), %src(s32), %zero(s32)
bool TargetInstructionLegalizer::ExpandZEXT(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "ZEXT must have exactly 2 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Src = *MI->GetOperand(1);

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert(Src.IsVirtualReg() && "Source must be a virtual register");

  auto Zero =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  MachineInstruction LI;
  LI.SetOpcode(MachineInstruction::LOAD_IMM);
  LI.AddOperand(Zero);
  LI.AddImmediate(0);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(LI), MI);

  auto Merge = MachineInstruction(MachineInstruction::MERGE, ParentBB);
  Merge.AddOperand(Dest);
  Merge.AddOperand(Src);
  Merge.AddOperand(Zero);
  ParentBB->InsertAfter(std::move(Merge), &*InsertTo);

  return true;
}

/// Expand 64 bit TRUNC to equivalent calculation using 32 bit instructions
///     TRUNC %dst(s32), %src(s64)
///
///  is replaced with
///     SPLIT      %src(s64), %lo(s32), %hi(s32)
///     TRUNC      %dst, %lo
bool TargetInstructionLegalizer::ExpandTRUNC(MachineInstruction *MI) {
  assert(MI->GetOperandsNumber() == 2 && "ZEXT must have exactly 2 operands");
  auto ParentBB = MI->GetParent();
  auto ParentFunc = ParentBB->GetParent();

  auto Dest = *MI->GetOperand(0);
  auto Src = *MI->GetOperand(1);

  assert(Dest.IsVirtualReg() && "Result must be a virtual register");
  assert(Src.IsVirtualReg() && "Source must be a virtual register");

  auto Lo32 =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());
  auto Hi32 =
      MachineOperand::CreateVirtualRegister(ParentFunc->GetNextAvailableVReg());

  auto Split = MachineInstruction(MachineInstruction::SPLIT, ParentBB);
  Split.AddOperand(Lo32);
  Split.AddOperand(Hi32);
  Split.AddOperand(Src);
  auto InsertTo = ParentBB->ReplaceInstr(std::move(Split), MI);

  auto Trunc = MachineInstruction(MachineInstruction::TRUNC, ParentBB);
  Trunc.AddOperand(Dest);
  Trunc.AddOperand(Lo32);
  ParentBB->InsertAfter(std::move(Trunc), &*InsertTo);

  return true;
}

bool TargetInstructionLegalizer::Expand(MachineInstruction *MI) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::ADD:
    return ExpandADD(MI);
  case MachineInstruction::ADDS:
    return ExpandADDS(MI);
  case MachineInstruction::ADDC:
    return ExpandADDC(MI);
  case MachineInstruction::XOR:
    return ExpandXOR(MI);
  case MachineInstruction::CMP:
    return ExpandCMP(MI);
  case MachineInstruction::MOD:
  case MachineInstruction::MODU:
    return ExpandMOD(MI, MI->GetOpcode() == MachineInstruction::MODU);
  case MachineInstruction::LOAD:
    return ExpandLOAD(MI);
  case MachineInstruction::LOAD_IMM:
    return ExpandLOAD_IMM(MI);
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
  case MachineInstruction::TRUNC:
    return ExpandTRUNC(MI);
  case MachineInstruction::GLOBAL_ADDRESS:
    return ExpandGLOBAL_ADDRESS(MI);
  default:
    break;
  }

  return false;
}
