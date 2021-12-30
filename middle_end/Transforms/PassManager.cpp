#include "PassManager.hpp"
#include "../IR/Module.hpp"
#include "../IR/Function.hpp"

bool PassManager::RunAll() {
  auto CSE = std::make_unique<CSEPass>();
  auto CopyProp = std::make_unique<CopyPropagationPass>();
  auto ValNum = std::make_unique<ValueNumberingPass>();
  auto LoopHoist = std::make_unique<LoopHoistingPass>();
  auto DCE = std::make_unique<DeadCodeEliminationPass>();
  

  for (auto &F : IRModule->GetFunctions()) {
    CSE->RunOnFunction(F);
    CopyProp->RunOnFunction(F);
    ValNum->RunOnFunction(F);
    LoopHoist->RunOnFunction(F);
    DCE->RunOnFunction(F);
  }

  return true;
}
