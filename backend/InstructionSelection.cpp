#include "InsturctionSelection.hpp"

void InsturctionSelection::InstrSelect() {
  for (auto &MFunc : MIRM->GetFunctions()) {
    for (auto &MBB : MFunc.GetBasicBlocks())
      for (auto &Instr : MBB.GetInstructions())
        TM->SelectInstruction(&Instr);
  }
}
