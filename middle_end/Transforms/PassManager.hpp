#ifndef PASS_MANAGER_HPP
#define PASS_MANAGER_HPP

#include "CSEPass.hpp"
#include "CopyPropagationPass.hpp"
#include "DeadCodeEliminationPass.hpp"
#include "LoopHoistingPass.hpp"
#include "ValueNumberingPass.hpp"
#include <set>

class Module;

enum Optimization {
  NONE,
  CopyPropagation,
  CSE,
};

class PassManager {
public:
  explicit PassManager(Module *m, std::set<Optimization> &opts)
      : IRModule(m), Optimizations(opts) {}

  bool RunAll();

private:
  Module *IRModule;
  std::set<Optimization> &Optimizations;
};

#endif // PASS_MANAGER_HPP
