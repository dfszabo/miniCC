#ifndef LOOP_HOISTING_PASS_HPP
#define LOOP_HOISTING_PASS_HPP

#include "FunctionPass.hpp"

class LoopHoistingPass : public FunctionPass {
public:
  virtual bool RunOnFunction(Function &F) override;
};

#endif // LOOP_HOISTING_PASS_HPP
