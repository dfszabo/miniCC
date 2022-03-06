#include "RegisterAllocator.hpp"
#include "MachineIRModule.hpp"
#include "MachineOperand.hpp"
#include "Support.hpp"
#include "TargetInstruction.hpp"
#include "TargetMachine.hpp"
#include "TargetRegister.hpp"
#include <algorithm>
#include <vector>
#include <set>
#include <iostream>

using VirtualReg = unsigned;
using PhysicalReg = unsigned;
using LiveRangeMap = std::map<VirtualReg, std::pair<unsigned, unsigned>>;

void PreAllocateParameters(MachineFunction &Func, TargetMachine *TM,
                           std::map<VirtualReg, PhysicalReg> &AllocatedRegisters,
                           LiveRangeMap &LiveRanges) {
  auto ArgRegs = TM->GetABI()->GetArgumentRegisters();
  unsigned CurrentParamReg = 0;

  for (auto [ParamID, ParamLLT, IsImplStructPtr, IsFP] : Func.GetParameters()) {
    // FIXME: excess parameters should be stored on the stack
    assert(CurrentParamReg < ArgRegs.size() && "Run out of param regs");

    // set the parameter live
    LiveRanges[ParamID] = {0, ~0};

    TargetRegister *CurrArgReg = nullptr;
    if (IsImplStructPtr)
        CurrArgReg = TM->GetRegInfo()->GetRegisterByID(
                              TM->GetRegInfo()->GetStructPtrRegister());
    else if (!IsFP)
      CurrArgReg = ArgRegs[CurrentParamReg++];
    else
      CurrArgReg =
          ArgRegs[TM->GetABI()->GetFirstFPArgRegIdx() + CurrentParamReg++];

    assert(CurrArgReg && "Cannot be null");

    // allocate the param to the CurrentParamReg -th param register
    if (ParamLLT.GetBitWidth() <= 32)
      AllocatedRegisters[ParamID] = CurrArgReg->GetSubRegs()[0];
    else
      AllocatedRegisters[ParamID] = CurrArgReg->GetID();
  }
}

void PreAllocateReturnRegister(
    MachineFunction &Func, TargetMachine *TM,
    std::map<VirtualReg, PhysicalReg> &AllocatedRegisters) {
  auto RetRegs = TM->GetABI()->GetReturnRegisters();

  for (auto MBBIt = Func.GetBasicBlocks().rbegin();
       MBBIt != Func.GetBasicBlocks().rend(); MBBIt++)
    for (auto It = MBBIt->GetInstructions().rbegin();
         It != MBBIt->GetInstructions().rend(); It++) {
      // If return instruction
      const auto Opcode = It->GetOpcode();
      if (auto TargetInstr = TM->GetInstrDefs()->GetTargetInstr(Opcode);
          TargetInstr->IsReturn()) {
        // if the ret has no operands it means the function ret type is void and
        // therefore does not need allocation for return registers
        if (It->GetOperandsNumber() == 0)
          continue;

        const auto RetValSize = It->GetOperands()[0].GetSize();

        // find the appropriate sized target register for the return value
        if (RetValSize == RetRegs[0]->GetBitWidth())
          AllocatedRegisters[It->GetOperand(0)->GetReg()] = RetRegs[0]->GetID();
        else // TODO: this is AArch64 specific
          AllocatedRegisters[It->GetOperand(0)->GetReg()] =
              RetRegs[0]->GetSubRegs()[0];
      }
    }
}

PhysicalReg GetNextAvailableReg(MachineOperand *MOperand,
                                std::set<PhysicalReg> &Pool,
                                std::set<PhysicalReg> &BackupPool,
                                TargetMachine *TM, MachineFunction &MFunc) {
  // TODO: implement spilling and remove this assertion then
  assert(!(Pool.empty() && BackupPool.empty()) && "Ran out of registers");

  if (Pool.empty()) {
    const auto BackupReg = *BackupPool.begin();
    Pool.insert(BackupReg);
    MFunc.GetUsedCalleSavedRegs().push_back(BackupReg);
    BackupPool.erase(BackupReg);
  }

  for (auto UnAllocatedReg : Pool) {
    // If the register class matches the requested operand's class, then return
    // this register and delete it from the pool
    if (TM->GetRegInfo()->GetRegClassFromReg(UnAllocatedReg) ==
        MOperand->GetRegClass()) {
      Pool.erase(UnAllocatedReg);
      return UnAllocatedReg;
    }
    // Otherwise check the subregisters of the register if it has, and try to
    // find a right candidate
    auto UnAllocatedRegInfo = TM->GetRegInfo()->GetRegisterByID(UnAllocatedReg);
    for (auto SubReg : UnAllocatedRegInfo->GetSubRegs()) {
      if (TM->GetRegInfo()->GetRegClassFromReg(SubReg) ==
          MOperand->GetRegClass()) {
        Pool.erase(UnAllocatedReg);
        return SubReg;
      }
    }
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
    std::set<PhysicalReg> RegisterPool;

    // Used if run out of caller saved registers
    std::set<PhysicalReg> BackupRegisterPool;

    // Initialize the usable register's pool
    for (auto TargetReg : TM->GetABI()->GetCallerSavedRegisters())
      RegisterPool.insert(TargetReg->GetID());

    // Initialize the backup register pool with the callee saved ones
    for (auto TargetReg : TM->GetABI()->GetCalleeSavedRegisters())
      BackupRegisterPool.insert(TargetReg->GetID());

    PreAllocateParameters(Func, TM, AllocatedRegisters, LiveRanges);
    PreAllocateReturnRegister(Func, TM, AllocatedRegisters);

    // Remove the pre allocated registers from the register pool
    std::set<PhysicalReg> RegsToBeRemoved;
    for (const auto [VirtReg, PhysReg] : AllocatedRegisters) {
      const auto ParentReg = TM->GetRegInfo()->GetParentReg(PhysReg);
      RegsToBeRemoved.insert(ParentReg ? ParentReg->GetID() : PhysReg);
    }

    // remove all the registers which are already allocated from the register
    // pool
    // FIXME: temporary simple solution for miss compiles caused by the RA
    // unaware of the liveranges of this registers
    for (auto &BB : Func.GetBasicBlocks())
      for (auto &Instr : BB.GetInstructions())
        for (size_t i = 0; i < Instr.GetOperandsNumber(); i++) {
          auto &Operand = Instr.GetOperands()[i];

          if (Operand.IsRegister()) {
            const auto PhysReg = Operand.GetReg();
            const auto ParentReg = TM->GetRegInfo()->GetParentReg(PhysReg);
            RegsToBeRemoved.insert(ParentReg ? ParentReg->GetID() : PhysReg);
          }
        }

    // Actually removing the registers from the pool
    for (auto Reg : RegsToBeRemoved)
        RegisterPool.erase(Reg);

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
    std::cout << std::endl;
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

                if (LDef < RDef)
                  return true;
                else if (LDef == RDef)
                  return LKill < RKill;
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

    // To keep track the already allocated, but not yet freed live ranges
    std::vector<std::tuple<unsigned, unsigned, unsigned>> FreeAbleWorkList;
    for (const auto &[VReg, DefLine, KillLine] : SortedLiveRanges) {

      // First free registers which are already killed at this point
      for (int i = 0; i < (int)FreeAbleWorkList.size(); i++) {
        auto [CheckVReg, CheckDefLine, CheckKillLine] = FreeAbleWorkList[i];
        // the above checked entry definitions line
        // is greater then this entry kill line. Meaning the register assigned
        // to this entry can be freed, since we already passed the line where it
        // was last used (killed)
        if (CheckKillLine < DefLine) {
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
          FreeAbleWorkList.erase(FreeAbleWorkList.begin() + i);
          i--; // to correct the index i, because of the erase
        }
      }

      // Then if this VReg is not allocated yet, then allocate it
      if (AllocatedRegisters.count(VReg) == 0) {
        AllocatedRegisters[VReg] =
            GetNextAvailableReg(VRegToMOMap[VReg], RegisterPool,
                                BackupRegisterPool, TM, Func);
        FreeAbleWorkList.push_back({VReg, DefLine, KillLine});
      }
#ifdef DEBUG
      std::cout << "VReg " << VReg << " allocated to "
                << TM->GetRegInfo()->GetRegisterByID(AllocatedRegisters[VReg])->GetName()
                << std::endl;
#endif
    }

#ifdef DEBUG
    std::cout << std::endl << std :: endl << "AllocatedRegisters" << std::endl;
    for (auto [VReg, PhysReg] : AllocatedRegisters)
      std::cout << "VReg: " << VReg << " to " <<
          TM->GetRegInfo()->GetRegisterByID(PhysReg)->GetName() << std::endl;
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
