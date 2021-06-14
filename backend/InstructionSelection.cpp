#include "InsturctionSelection.hpp"

void InsturctionSelection::InstrSelect() {
  for (auto &MFunc : MIRM->GetFunctions()) {
    for (auto &MBB : MFunc.GetBasicBlocks())
      for (auto &Instr : MBB.GetInstructions())
        // Skip selection if already selected
        if (!Instr.IsAlreadySelected())
          TM->SelectInstruction(&Instr);
  }
}
