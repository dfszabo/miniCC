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
    auto Opcode = It->GetOpcode();
    if (auto TargetInstr = TM->GetInstrDefs()->GetTargetInstr(Opcode);
        TargetInstr->IsReturn())
      AllocatedRegisters[It->GetOperand(0)->GetReg()] = RetRegs[0]->GetID();
  }
}

void RegisterAllocator::RunRA() {
  // mapping from virtual reg to physical
  std::map<unsigned, unsigned> AllocatedRegisters;
  std::vector<unsigned> RegisterPool;  // available registers
  std::set<unsigned> RegistersToSpill; // register require spilling

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

    // we want to keep track of how many consecutive renames happened since only
    // two can be afforded for now, doing more then that is an error
    unsigned ConsecutiveLoadRenames = 0;
    unsigned ConsecutiveStoreRenames = 0;

    for (auto &BB : Func.GetBasicBlocks())
      for (auto &Instr : BB.GetInstructions())
        for (auto &Operand : Instr.GetOperands())
          if (Operand.IsVirtualReg() || Operand.IsParameter()) {
            const auto UsedReg = Operand.GetReg();
            bool AlreadyAllocated = 0 != AllocatedRegisters.count(UsedReg);

            // when short on allocatable registers, then add the register
            // to the spillable set if its not a load or a store
            if (!AlreadyAllocated && RegisterPool.size() <= 3) {
              // if the instruction is NOT a load, then spill it
              if (!Instr.IsLoadOrStore()) {
                ConsecutiveLoadRenames = ConsecutiveStoreRenames = 0;
                RegistersToSpill.insert(UsedReg);
                continue;
              }

              // Depending on whether its a load or a store, rename the register
              // (allocating essentially)
              if (Instr.IsLoad()) {
                // Note that the last 2 register is used for this purpose
                AllocatedRegisters[UsedReg] =
                    RegisterPool.rbegin()[1 - ConsecutiveLoadRenames];
                ConsecutiveLoadRenames++;
              } else {
                AllocatedRegisters[UsedReg] = RegisterPool.rbegin()[2];
                ConsecutiveStoreRenames++;
              }

              // if its a load then rename it, which means to allocate it one of
              // the registers used for loading spilled regs
              assert(ConsecutiveLoadRenames <= 2 &&
                     "To much consecutive loads to rename");
              assert(ConsecutiveStoreRenames <= 1 &&
                     "To much consecutive stores to rename");

              AlreadyAllocated = true;
            }

            // if not allocated yet
            if (!AlreadyAllocated) {
              // then allocate it
              auto Reg = RegisterPool[0];
              AllocatedRegisters[UsedReg] = Reg;
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
              Operand.SetReg(AllocatedRegisters[UsedReg]);
            }
          }

    ///////////// Handle Spills

    // if there is nothing to spill then nothing todo
    if (RegistersToSpill.size() == 0)
      continue;

    for (auto Reg : RegistersToSpill)
      // TODO: Size should be the spilled register size, not hardcoded
      Func.InsertStackSlot(Reg, 4);

    for (auto &BB : Func.GetBasicBlocks()) {
      auto &Instructions = BB.GetInstructions();

      for (size_t i = 0; i < Instructions.size(); i++) {

        // Check operands which use the register (the first operand defining the
        // reg the rest uses regs)
        for (size_t OpIndex = 1; OpIndex < Instructions[i].GetOperands().size();
             OpIndex++) {
          auto &Operand = Instructions[i].GetOperands()[OpIndex];
          auto UsedReg = Operand.GetReg();

          // if the used register is not spilled, then nothing to do
          if (RegistersToSpill.count(UsedReg) == 0)
            continue;

          ////// Insert a LOAD

          // First make the operand into a physical register which using
          // the register used for the spilling
          Operand.SetToRegister();
          Operand.SetReg(RegisterPool[OpIndex]);

          auto Load = MachineInstruction(MachineInstruction::LOAD, &BB);
          /// NOTE: 3 register left in pool using the 2nd and 3rd for uses
          Load.AddRegister(RegisterPool[OpIndex]);
          Load.AddStackAccess(UsedReg);
          TM->SelectInstruction(&Load);

          BB.InsertInstr(std::move(Load), i++);
        }

        /// Insert STORE if needed
        auto &Operand = Instructions[i].GetOperands()[0];
        auto DefReg = Operand.GetReg();

        if (RegistersToSpill.count(DefReg) == 0)
          continue;

        Operand.SetToRegister();
        Operand.SetReg(RegisterPool[0]);

        auto Store = MachineInstruction(MachineInstruction::STORE, &BB);

        Store.AddStackAccess(DefReg);
        /// NOTE: 3 register left in pool using the 1st for def
        Store.AddRegister(RegisterPool[0]);
        TM->SelectInstruction(&Store);

        BB.InsertInstr(std::move(Store), ++i);
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

          // Using SP as frame register for simplicity
          // TODO: Add FP register handling if target support it.
          auto FrameReg = TM->GetRegInfo()->GetStackRegister();
          auto Offset = StackFrameSize - 4 -
                        (int)Func.GetStackObjectPosition(Operand.GetSlot());

          Instr.RemoveMemOperand();
          Instr.AddRegister(FrameReg);
          Instr.AddImmediate(Offset);

          break; // there should be only at most one stack access / instr
        }
      }
  }
}
