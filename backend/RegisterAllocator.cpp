#include "RegisterAllocator.hpp"
#include "MachineIRModule.hpp"
#include "MachineOperand.hpp"
#include "Support.hpp"
#include "TargetInstruction.hpp"
#include "TargetMachine.hpp"
#include "TargetRegister.hpp"
#include <algorithm>
#include <vector>
#include <iostream>

using VirtualReg = unsigned;
using PhysicalReg = unsigned;
using LiveRangeMap = std::map<VirtualReg, std::pair<unsigned, unsigned>>;

void PreAllocateParameters(MachineFunction &Func, TargetMachine *TM,
                           std::map<VirtualReg, PhysicalReg> &AllocatedRegisters,
                           LiveRangeMap &LiveRanges) {
  auto ArgRegs = TM->GetABI()->GetArgumentRegisters();
  unsigned CurrentParamReg = 0;

  for (auto [ParamID, ParamLLT] : Func.GetParameters()) {
    // FIXME: excess parameters should be stored on the stack
    assert(CurrentParamReg < ArgRegs.size() && "Run out of param regs");

    // set the parameter live
    LiveRanges[ParamID] = {0, ~0};

    // allocate the param to the CurrentParamReg -th param register
    if (ParamLLT.GetBitWidth() <= 32)
      AllocatedRegisters[ParamID] = ArgRegs[CurrentParamReg++]->GetSubRegs()[0];
    else
      AllocatedRegisters[ParamID] = ArgRegs[CurrentParamReg++]->GetID();
  }
}

void PreAllocateReturnRegister(
    MachineFunction &Func, TargetMachine *TM,
    std::map<VirtualReg, PhysicalReg> &AllocatedRegisters) {
  auto RetRegs = TM->GetABI()->GetReturnRegisters();
  auto LastBBInstrs = Func.GetBasicBlocks().back().GetInstructions();

  for (auto It = LastBBInstrs.rbegin(); It != LastBBInstrs.rend(); It++) {
    // If return instruction
    auto Opcode = It->GetOpcode();
    if (auto TargetInstr = TM->GetInstrDefs()->GetTargetInstr(Opcode);
        TargetInstr->IsReturn()) {
      auto RetValSize = It->GetOperands()[0].GetSize();

      if (RetValSize == RetRegs[0]->GetBitWidth())
        AllocatedRegisters[It->GetOperand(0)->GetReg()] = RetRegs[0]->GetID();
      else
        AllocatedRegisters[It->GetOperand(0)->GetReg()] = RetRegs[0]->GetSubRegs()[0];
    }
  }
}

PhysicalReg GetNextAvailableReg(uint8_t BitSize, std::vector<PhysicalReg> &Pool,
                                TargetMachine *TM) {
  unsigned loopCounter = 0;
  for (auto UnAllocatedReg : Pool) {
    auto UnAllocatedRegInfo = TM->GetRegInfo()->GetRegisterByID(UnAllocatedReg);
    // If the register bit width matches the requested size then return this
    // register and delete it from the pool
    if (UnAllocatedRegInfo->GetBitWidth() == BitSize) {
      Pool.erase(Pool.begin() + loopCounter);
      return UnAllocatedReg;
    }
    // Otherwise check the subregisters of the register if it has, and try to
    // find a right candidate
    for (auto SubReg : UnAllocatedRegInfo->GetSubRegs()) {
      auto SubRegInfo = TM->GetRegInfo()->GetRegisterByID(SubReg);
      if (SubRegInfo->GetBitWidth() == BitSize) {
        Pool.erase(Pool.begin() + loopCounter);
        return SubReg;
      }
    }

    loopCounter++;
  }

  assert(!"Have not found the right registers");
  return 0;
}

// TODO: Add handling for spilling registers
void RegisterAllocator::RunRA() {
  for (auto &Func : MIRM->GetFunctions()) {
    // mapping virtual registers to live ranges, where the live range represent
    // the pair of the first definition (def) of the virtual register and the
    // last use (kill) of it. Kill initialized to ~0 to signal errors
    // potentially dead regs in the future
    LiveRangeMap LiveRanges;
    std::map<VirtualReg, MachineOperand*> VRegToMOMap;
    std::map<VirtualReg, PhysicalReg> AllocatedRegisters;
    std::vector<PhysicalReg> RegisterPool;

    // Initialize the usable register's pool
    for (auto TargetReg : TM->GetABI()->GetCallerSavedRegisters())
      RegisterPool.push_back(TargetReg->GetID());

    PreAllocateParameters(Func, TM, AllocatedRegisters, LiveRanges);
    PreAllocateReturnRegister(Func, TM, AllocatedRegisters);

    // Remove the pre allocated registers from the register pool
    for (const auto [VirtReg, PhysReg] : AllocatedRegisters) {
      auto RegsToCheck = TM->GetRegInfo()->GetRegisterByID(PhysReg)->GetSubRegs();
      RegsToCheck.push_back(PhysReg);
      auto ParentReg = TM->GetRegInfo()->GetParentReg(PhysReg);
      if (ParentReg)
        RegsToCheck.push_back(ParentReg->GetID());

      for (auto Reg : RegsToCheck) {
        auto position = std::find(RegisterPool.begin(), RegisterPool.end(),
                                  Reg);
        if (position != RegisterPool.end())
          RegisterPool.erase(position);
      }
    }

    // Calculating the live ranges for the virtual registers
    unsigned InstrCounter = 0;
    for (auto &BB : Func.GetBasicBlocks())
      for (auto &Instr : BB.GetInstructions()) {
        for (size_t i = 0; i < Instr.GetOperandsNumber(); i++) {
          auto &Operand = Instr.GetOperands()[i];

          if (Operand.IsVirtualReg() || Operand.IsParameter() ||
              Operand.IsMemory()) {
            auto UsedReg = Operand.GetReg();
            // Save the VReg Operand into a map to be able to look it up later
            // for size information like its bit size
            if (VRegToMOMap.count(UsedReg) == 0)
              VRegToMOMap[UsedReg] = &Instr.GetOperands()[i];

            // if this VirtualReg first encountered
            // for now assuming also its a definition if we encountered it first
            if (LiveRanges.count(UsedReg) == 0)
              LiveRanges[UsedReg] = {InstrCounter, ~0};

            // otherwise it was already seen (therefore defined) so we only
            // have to update the LiveRange entry last use part
            else
              LiveRanges[UsedReg].second = InstrCounter;

          }
        } // Operand end
        InstrCounter++;
      } // Instr end

#ifdef DEBUG
    for (const auto &[VReg, LiveRange] : LiveRanges) {
      auto [DefLine, KillLine] = LiveRange;
      std::cout << "VReg: " << VReg << ", LiveRange(" << DefLine << ", "
                << KillLine << ")" << std::endl;
    }
#endif

    // make a sorted vector from the map where the element ordered by the
    // LiveRange kill field, if both kill field is equal then the def field will
    // decide it
    std::vector<std::tuple<unsigned, unsigned, unsigned>> SortedLiveRanges;
    for (const auto &[VReg, LiveRange] : LiveRanges) {
      auto [DefLine, KillLine] = LiveRange;
      SortedLiveRanges.push_back({VReg, DefLine, KillLine});
    }
    std::sort (SortedLiveRanges.begin(), SortedLiveRanges.end(),
              [](std::tuple<unsigned, unsigned, unsigned> Left,
                 std::tuple<unsigned, unsigned, unsigned> Right) {
                auto [LVReg, LDef, LKill] = Left;
                auto [RVReg, RDef, RKill] = Right;

                if (LKill < RKill)
                  return true;
                else if (LKill == RKill)
                  return LDef < RDef;
                else
                  return false;
    });

#ifdef DEBUG
    std::cout << "SortedLiveRanges" << std::endl;
    for (const auto &[VReg, DefLine, KillLine] : SortedLiveRanges)
      std::cout << "VReg: " << VReg << ", LiveRange(" << DefLine << ", "
                << KillLine << ")" << std::endl;
    std::cout << std::endl;
#endif

    // Where to start the search from registers to be freed. Entries with
    // smaller indexes were already freed, therefore don't have to check them.
    unsigned StartIndex = 0;
    unsigned CurrentIdx = 0;
    for (const auto &[VReg, DefLine, KillLine] : SortedLiveRanges) {
      // if not allocated yet, then allocate it
      if (AllocatedRegisters.count(VReg) == 0)
        AllocatedRegisters[VReg] =
            GetNextAvailableReg(VRegToMOMap[VReg]->GetSize(), RegisterPool, TM);

      // free registers which are already killed
      unsigned CheckCurrentIdx = 0;
      for (const auto [CheckVReg, CheckDefLine, CheckKillLine] :
           SortedLiveRanges) {
        // only check for the current and the previous entries
        if (CheckCurrentIdx >= CurrentIdx)
          break;

        // only start doing something when the start index of the non freed
        // entries reached first and the above checked entry definitions line
        // is greater then this entry kill line. Meaning the register assigned
        // to this entry can be freed, since we already passed the line where it
        // was last used (killed)
        if (CheckCurrentIdx >= StartIndex && CheckKillLine < DefLine) {
          // Freeing the register allocated to this live range's register
          // If its a subregister then we have to find its parent first and then
          // put that back to the allocatable register's RegisterPool
          assert(AllocatedRegisters.count(CheckVReg) > 0);
          unsigned FreeAbleReg = AllocatedRegisters[CheckVReg];
          auto ParentReg = TM->GetRegInfo()->GetParentReg(FreeAbleReg);
          if (ParentReg)
            FreeAbleReg = ParentReg->GetID();

#ifdef DEBUG
          std::cout << "Freed register "
                    << TM->GetRegInfo()->GetRegisterByID(FreeAbleReg)->GetName()
                    << std::endl;
#endif
          RegisterPool.insert(RegisterPool.begin(), FreeAbleReg);
          StartIndex++;
        }
        CheckCurrentIdx++;
      }
      CurrentIdx++;
    }

#ifdef DEBUG
    std::cout << std::endl << std :: endl << "AllocatedRegisters" << std::endl;
    for (auto [VReg, PhysReg] : AllocatedRegisters)
      std::cout << "VReg: " << VReg << " to " <
          < TM->GetRegInfo()->GetRegisterByID(PhysReg)->GetName() << std::endl;
    std::cout << std::endl << std::endl;
#endif

    // Setting to operands from virtual register to register as a last part of
    // the allocation
    for (auto &BB : Func.GetBasicBlocks())
      for (auto &Instr : BB.GetInstructions())
        for (size_t i = 0; i < Instr.GetOperandsNumber(); i++) {
          auto &Operand = Instr.GetOperands()[i];
          auto PhysReg = AllocatedRegisters[Operand.GetReg()];
          if (Operand.IsVirtualReg() || Operand.IsParameter()) {
            Operand.SetToRegister();
            Operand.SetReg(PhysReg);
          } else if (Operand.IsMemory()) {
            Operand.SetVirtual(false);
            Operand.SetValue(PhysReg);
          }
        }

    // FIXME: Move this out from here and make it a PostRA pass
    // After RA lower the stack accessing operands to their final form
    // based on the final stack frame
    for (auto &BB : Func.GetBasicBlocks())
      for (auto &Instr : BB.GetInstructions()) {
        // Check the operands
        for (auto &Operand : Instr.GetOperands()) {
          // Only interested in memory accessing operands
          if (!Operand.IsStackAccess() && !Operand.IsMemory())
            continue;

          // Handle stack access
          if (Operand.IsStackAccess()) {
            // Using SP as frame register for simplicity
            // TODO: Add FP register handling if target support it.
            auto FrameReg = TM->GetRegInfo()->GetStackRegister();
            auto Offset = (int)Func.GetStackObjectPosition(Operand.GetSlot())
                          + Operand.GetOffset();

            Instr.RemoveMemOperand();
            Instr.AddRegister(FrameReg, TM->GetPointerSize());
            Instr.AddImmediate(Offset);
          }
          // Handle memory access
          else {
            auto BaseReg = Operand.GetReg();
            // TODO: Investigate when exactly this should be other then 0
            auto Offset = Operand.GetOffset();

            unsigned Reg =
                Operand.IsVirtual() ? AllocatedRegisters[BaseReg] : BaseReg;

            auto RegSize = TM->GetRegInfo()->GetRegister(Reg)->GetBitWidth();
            Instr.RemoveMemOperand();
            Instr.AddRegister(Reg, RegSize);
            Instr.AddImmediate(Offset);
          }

          break; // there should be only at most one stack access / instr
        }
      }
  } // Func end
}
