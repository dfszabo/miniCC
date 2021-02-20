#include "RegisterAllocator.hpp"
#include "MachineIRModule.hpp"
#include "MachineOperand.hpp"
#include "Support.hpp"
#include "TargetInstruction.hpp"
#include "TargetMachine.hpp"
#include "TargetRegister.hpp"
#include <algorithm>
#include <vector>

void PreAllocateParameters(MachineFunction &Func, TargetMachine *TM,
                           std::map<unsigned, unsigned> &AllocatedRegisters) {
  auto ArgRegs = TM->GetABI()->GetArgumentRegisters();
  unsigned CurrentParamReg = 0;

  for (auto Param : Func.GetParameters()) {
    // FIXME: Handle others as well
    assert(Param.second.GetBitWidth() <= 32 &&
           "Only handling <= 32 bits for now");
    // FIXME: excess parameters should be stored on the stack
    assert(CurrentParamReg < ArgRegs.size() && "Run out of param regs");
    // allocate the param to the CurrentParamReg -th param register
    AllocatedRegisters[Param.first] = ArgRegs[CurrentParamReg++]->GetID();
  }
}

void PreAllocateReturnRegister(
    MachineFunction &Func, TargetMachine *TM,
    std::map<unsigned, unsigned> &AllocatedRegisters) {
  auto RetRegs = TM->GetABI()->GetReturnRegisters();
  auto LastBBInstrs = Func.GetBasicBlocks().back().GetInstructions();

  for (auto It = LastBBInstrs.rbegin(); It != LastBBInstrs.rend(); It++) {
    // If return instruction
    if (auto TargetInstr = TM->GetInstrDefs()->GetTargetInstr(It->GetOpcode());
        TargetInstr->IsReturn())
      AllocatedRegisters[It->GetOperand(0)->GetReg()] = RetRegs[0]->GetID();
  }
}

void RegisterAllocator::RunRA() {
  // mapping from virtual reg to physical
  std::map<unsigned, unsigned> AllocatedRegisters;
  std::vector<unsigned> RegisterPool; // available registers

  // Initialize the usable register's pool
  for (auto TargetReg : TM->GetABI()->GetCallerSavedRegisters())
    RegisterPool.push_back(TargetReg->GetID());

  for (auto &Func : MIRM->GetFunctions()) {
    PreAllocateParameters(Func, TM, AllocatedRegisters);
    PreAllocateReturnRegister(Func, TM, AllocatedRegisters);

    // Remove the pre allocated registers from the register pool
    for (const auto &AllocRegEntry : AllocatedRegisters) {
      auto position = std::find(RegisterPool.begin(), RegisterPool.end(),
                                AllocRegEntry.second);
      if (position != RegisterPool.end())
        RegisterPool.erase(position);
    }

    for (auto &BB : Func.GetBasicBlocks())
      for (auto &Instr : BB.GetInstructions())
        for (auto &Operand : Instr.GetOperands())
          if (Operand.IsVirtualReg() || Operand.IsParameter()) {
            // should be runtime error in the future OR better: implement
            // spilling
            assert(RegisterPool.size() > 0 && "Ran out of registers");

            // if not allocated yet
            if (0 == AllocatedRegisters.count(Operand.GetReg())) {
              // then allocate it
              auto Reg = RegisterPool[0];
              AllocatedRegisters[Operand.GetReg()] = Reg;
              // FIXME: vector is not a good container type if we commonly
              // removing the first element HINT: list or dequeue
              // Remove the register from the available register pool
              RegisterPool.erase(RegisterPool.begin());

              Operand.SetToRegister();
              Operand.SetReg(Reg);
            }
            // else its already allocated so just look it up
            else {
              Operand.SetToRegister();
              Operand.SetReg(AllocatedRegisters[Operand.GetReg()]);
            }
          }
  }

  // FIXME: Move this out from here and make it a PostRA pass
  // After RA lower the stack asccessing operands to their final form
  // based on the final stack frame
  for (auto &Func : MIRM->GetFunctions()) {
    unsigned StackFrameSize = Func.GetStackFrameSize();
    StackFrameSize =
        GetNextAlignedValue(StackFrameSize, TM->GetABI()->GetStackAlignment());

    for (auto &BB : Func.GetBasicBlocks())
      for (auto &Instr : BB.GetInstructions()) {
        // Only consider load and stores
        auto TargetInstr =
            TM->GetInstrDefs()->GetTargetInstr(Instr.GetOpcode());
        if (!TargetInstr->IsLoadOrStore())
          continue;

        // Check the operands
        for (auto &Operand : Instr.GetOperands()) {
          // Only interested in stack accessing operands
          if (!Operand.IsStackAccess())
            continue;

          MachineOperand FrameRegister;
          MachineOperand Offset;

          // Using SP as frame register for simplicity
          // TODO: Add FP register handling if target support it.
          FrameRegister.SetToRegister();
          FrameRegister.SetReg(TM->GetRegInfo()->GetStackRegister());

          Offset.SetToIntImm();
          Offset.SetValue(StackFrameSize - 4 -
                          (int)Func.GetStackObjectPosition(Operand.GetSlot()));

          Instr.RemoveMemOperand();
          Instr.AddOperand(FrameRegister);
          Instr.AddOperand(Offset);

          break; // there should be only at most one stack access / instr
        }
      }
  }
}
