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
    for (size_t BBIndex = 0; BBIndex < Func.GetBasicBlocks().size(); BBIndex++)
      for (size_t InstrIndex = 0;
           InstrIndex < Func.GetBasicBlocks()[BBIndex].GetInstructions().size();
           InstrIndex++) {
        auto *MI =
            &Func.GetBasicBlocks()[BBIndex].GetInstructions()[InstrIndex];

        // If the instruction is not legal on the target
        if (!Legalizer->Check(MI)) {
          // but if it is expandable to hopefully legal ones, then do it
          if (Legalizer->IsExpandable(MI)) {
            Legalizer->Expand(MI);
            InstrIndex--;
            continue;
          } else {
            assert(!"Machine Instruction is not legal neither expandable");
          }
        }
      }
  }
}
