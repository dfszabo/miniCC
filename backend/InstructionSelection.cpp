#include "InsturctionSelection.hpp"

void InsturctionSelection::InstrSelect() {
  for (auto &MFunc : MIRM->GetFunctions()) {
    for (auto &MBB : MFunc.GetBasicBlocks())
      for (size_t i = 0; i < MBB.GetInstructions().size(); i++)
        // Skip selection if already selected
        if (!MBB.GetInstructions()[i].IsAlreadySelected()) {
          TM->SelectInstruction(&MBB.GetInstructions()[i]);
          assert(MBB.GetInstructions()[i].IsAlreadySelected());
        }
  }
}
