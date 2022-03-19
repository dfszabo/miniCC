#include "PassManager.hpp"
#include "../IR/Function.hpp"
#include "../IR/Module.hpp"

bool PassManager::RunAll() {
  auto CSE = std::make_unique<CSEPass>();
  auto CopyProp = std::make_unique<CopyPropagationPass>();
  auto ValNum = std::make_unique<ValueNumberingPass>();
  auto LoopHoist = std::make_unique<LoopHoistingPass>();
  auto DCE = std::make_unique<DeadCodeEliminationPass>();

  for (auto &F : IRModule->GetFunctions()) {
    if (Optimizations.count(Optimization::CopyPropagation) != 0) {
      CopyProp->RunOnFunction(F);
      DCE->RunOnFunction(F);
    }

    // CSE assumes copy propagation happened already
    if (Optimizations.count(Optimization::CopyPropagation) != 0 &&
        Optimizations.count(Optimization::CSE) != 0) {
      size_t InstNumAtStart;

      // It is an iterative process, since after CSE and DCE
      // now opportunities for CSE could arise, so running it until
      // there is no change in the instructions size between the
      // start and end of iteration.
      do {
        InstNumAtStart = F.GetNumberOfInstructions();
        CSE->RunOnFunction(F);
        DCE->RunOnFunction(F);
      } while (InstNumAtStart != F.GetNumberOfInstructions());
    }

    ValNum->RunOnFunction(F);
    LoopHoist->RunOnFunction(F);
    DCE->RunOnFunction(F);
  }

  return true;
}
