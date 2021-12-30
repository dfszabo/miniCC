#ifndef PASS_MANAGER_HPP
#define PASS_MANAGER_HPP

#include "CSEPass.hpp"
#include "CopyPropagationPass.hpp"
#include "DeadCodeEliminationPass.hpp"
#include "LoopHoistingPass.hpp"
#include "ValueNumberingPass.hpp"

class Module;

class PassManager {
public:
  PassManager(Module *m) : IRModule(m) {}

  bool RunAll();

private:
  Module *IRModule;
};

#endif // PASS_MANAGER_HPP
