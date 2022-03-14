#include "Module.hpp"
#include "BasicBlock.hpp"
#include "Function.hpp"
#include "Value.hpp"
#include <algorithm>
#include <cassert>

BasicBlock *Module::CurrentBB() {
  assert(!Functions.empty() && "Module must have functions.");
  return Functions.back().GetCurrentBB();
}

BasicBlock *Module::GetBB(const size_t Index) {
  assert(!Functions.empty() && "Module must have functions.");
  return Functions.back().GetBB(Index);
}

Function *Module::CurrentFunction() {
  assert(!Functions.empty() && "Module must have functions.");
  return &Functions.back();
}

void Module::AddFunction(Function F) { Functions.push_back(std::move(F)); }

void Module::AddGlobalVar(std::unique_ptr<Value> GV) {
  assert(GV && "Cannot be a nullptr");
  GlobalVars.push_back(std::move(GV));
}

bool Module::IsGlobalValue(Value *V) const {
  for (auto &GV : GlobalVars)
    if (GV.get() == V)
      return true;

  return false;
}

Value *Module::GetGlobalVar(const std::string &Name) const {
  for (auto &GV : GlobalVars)
    if (((GlobalVariable *)GV.get())->GetName() == Name)
      return GV.get();

  return nullptr;
}

BasicBlock *Module::CreateBasicBlock() {
  assert(!Functions.empty() && "Module must have functions.");
  Functions.back().CreateBasicBlock();
  return CurrentBB();
}

void Module::Print() const {
  for (auto &GlobalVar : GlobalVars)
    dynamic_cast<GlobalVariable *>(GlobalVar.get())->Print();
  for (auto &Function : Functions)
    Function.Print();
}
