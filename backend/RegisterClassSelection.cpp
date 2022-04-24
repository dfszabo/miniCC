#include "RegisterClassSelection.hpp"
#include <cassert>

bool IsFPInstruction(MachineInstruction *MI, size_t idx) {
  switch (MI->GetOpcode()) {
  case MachineInstruction::ADDF:
  case MachineInstruction::SUBF:
  case MachineInstruction::MULF:
  case MachineInstruction::DIVF:
  case MachineInstruction::MOVF:
    return true;

  case MachineInstruction::CMPF:
    return idx != 0; // the result is boolean and not fp

  case MachineInstruction::ITOF:
    return idx == 0; // the result is fp, the operand is integer

  case MachineInstruction::FTOI:
    return idx != 0; // the result is integer, the operand is fp

  default:
    return false;
  }
}

void RegisterClassSelection::Run() {
  // To store the register class of the stored registers to the stack
  std::map<unsigned, unsigned> StackSlotToRegClass;

  // To store already processed virtual register's register class
  std::map<unsigned, unsigned> VRegToRegClass;

  unsigned ParameterCounter;

  for (auto &MFunc : MIRM->GetFunctions()) {
    // reset
    StackSlotToRegClass.clear();
    VRegToRegClass.clear();
    ParameterCounter = 0;

    for (auto &MBB : MFunc.GetBasicBlocks())
      for (size_t i = 0; i < MBB.GetInstructions().size(); i++)
        for (size_t op_idx = 0;
             op_idx < MBB.GetInstructions()[i].GetOperandsNumber(); op_idx++) {
          MachineInstruction *MI = &MBB.GetInstructions()[i];
          MachineOperand *Op = MI->GetOperand(op_idx);

          // if it is a store instruction accessing the stack, then map the
          // stack slot to the appropriate register class
          if (Op->IsStackAccess() && MI->IsStore() &&
              StackSlotToRegClass.count(Op->GetSlot()) == 0 &&
              MI->GetOperandsNumber() > op_idx + 1) {
            assert(MI->GetOperandsNumber() > op_idx + 1);
            auto *NextOp = MI->GetOperand(op_idx + 1);

            // if the next operand is a virtual register which class is
            // already determined
            if (NextOp->IsVirtualReg() &&
                VRegToRegClass.count(NextOp->GetReg())) {
              StackSlotToRegClass[Op->GetSlot()] =
                  VRegToRegClass[NextOp->GetReg()];
              continue;
            } 
            // if it is a physical register, then ask the target for which
            // register class this register belongs to
            else if (NextOp->IsRegister()) {
              StackSlotToRegClass[Op->GetSlot()] =
                  TM->GetRegInfo()->GetRegClassFromReg(NextOp->GetReg());
              continue;
            }
            // if it is a parameter, then use the function's parameter info to
            // determine the appropriate register class
            else if (NextOp->IsParameter()) {
              auto [ParamNum, Type, IsStructPtr, IsFP] =
                  MFunc.GetParameters()[ParameterCounter++];
              unsigned RC =
                  TM->GetRegInfo()->GetRegisterClass(Type.GetBitWidth(), IsFP);
              StackSlotToRegClass[Op->GetSlot()] = RC;
            }
          }

          // if it is a load instruction accessing the stack, then map the
          // stack slot to the appropriate register class
          if (Op->IsVirtualReg() && MI->IsLoad() &&
              MI->GetOperandsNumber() > op_idx) {
            auto *NextOp = MI->GetOperand(op_idx + 1);

            if (NextOp->IsStackAccess() &&
                StackSlotToRegClass.count(NextOp->GetReg())) {
              VRegToRegClass[Op->GetReg()] =
                  StackSlotToRegClass[NextOp->GetSlot()];
              Op->SetRegClass(VRegToRegClass[Op->GetReg()]);
              continue;
            }
          }

          // at this point only interested in virtual registers
          if (!Op->IsVirtual())
            continue;

          auto Reg = Op->GetReg();

          // check if this virtual register is already encountered
          if (VRegToRegClass.count(Reg)) {
            Op->SetRegClass(VRegToRegClass[Reg]);
            continue;
          }

          const bool IsFP = IsFPInstruction(&MBB.GetInstructions()[i], op_idx);
          unsigned RC = TM->GetRegInfo()->GetRegisterClass(Op->GetSize(), IsFP);
          assert(TM->GetRegInfo()->GetRegClassRegsSize(RC) >= Op->GetSize());
          Op->SetRegClass(RC);

          VRegToRegClass[Reg] = RC;
        }
  }
}
