#include "MachineInstructionLegalizer.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "TargetMachine.hpp"

void MachineInstructionLegalizer::Run() {
  auto Legalizer = TM->GetLegalizer();

  if (Legalizer == nullptr)
    return;

  for (auto &Func : MIRM->GetFunctions()) {
    for (size_t BBIndex = 0; BBIndex < Func.GetBasicBlocks().size();
         BBIndex++) {
      for (size_t InstrIndex = 0;
           InstrIndex < Func.GetBasicBlocks()[BBIndex].GetInstructions().size();
           InstrIndex++) {
        auto *MI =
            &Func.GetBasicBlocks()[BBIndex].GetInstructions()[InstrIndex];

        // If the instruction is not legal on the target and not selected yet
        // and has not yet been expanded
        if (!Legalizer->Check(MI) && !MI->IsAlreadySelected() &&
            !MI->IsAlreadyExpanded()) {
          // but if it is expandable to hopefully legal ones, then do it
          if (Legalizer->IsExpandable(MI)) {
            if (Legalizer->Expand(MI)) {
              InstrIndex--;
            } else {
              Legalizer->Expand(MI);
              assert(!"Expandable instruction should be expandable");
            }
            continue;
          } else {
            assert(!"Machine Instruction is not legal neither expandable");
          }
        }
      }

      // After processing the BB propagate SPLIT and MERGE instruction
      // registers and remove these instructions
      auto &Instructions = Func.GetBasicBlocks()[BBIndex].GetInstructions();
      std::map<uint64_t, std::pair<uint64_t, uint64_t>> MergedValuesMap;
      std::map<uint64_t, uint64_t> RegisterMap;
      for (size_t InstrIndex = 0; InstrIndex < Instructions.size();
           InstrIndex++) {

        auto *MI = &Instructions[InstrIndex];

        // If it is a merge then register its operands and delete it
        if (MI->IsMerge()) {
          assert(MI->GetOperandsNumber() == 3);

          MergedValuesMap[MI->GetOperand(0)->GetReg()] = {
              MI->GetOperand(1)->GetReg(), MI->GetOperand(2)->GetReg()};
          Instructions.erase(Instructions.begin() + InstrIndex);
          InstrIndex--;
          continue;
        }

        // If a split is found
        else if (MI->IsSplit()) {
          assert(MI->GetOperandsNumber() == 3);
          assert(MergedValuesMap.count(MI->GetOperand(2)->GetReg()) > 0 &&
                 "The split source has not been defined by a merge yet");

          auto [Lo, Hi] = MergedValuesMap[MI->GetOperand(2)->GetReg()];

          const uint64_t SplitLo = MI->GetOperand(0)->GetReg();
          const uint64_t SplitHi = MI->GetOperand(1)->GetReg();

          RegisterMap[SplitLo] = Lo;
          RegisterMap[SplitHi] = Hi;

          Instructions.erase(Instructions.begin() + InstrIndex);
          InstrIndex--;
          continue;
        }

        // Else it some other kind of instruction, in that case check its
        // operands and map them
        for (size_t OpIdx = 0; OpIdx < MI->GetOperandsNumber(); OpIdx++) {
          // Only check virtual register operands
          if (!MI->GetOperand(OpIdx)->IsVirtual())
            continue;

          // If not mapped then skip
          if (RegisterMap.count(MI->GetOperand(OpIdx)->GetReg()) == 0)
            continue;

          MI->GetOperand(OpIdx)->SetReg(
              RegisterMap[MI->GetOperand(OpIdx)->GetReg()]);
        }
      }
    }
  }
}
