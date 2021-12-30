#ifndef COPY_PROPAGATION_PASS_HPP
#define COPY_PROPAGATION_PASS_HPP

#include "FunctionPass.hpp"

class CopyPropagationPass : public FunctionPass {
public:
  virtual bool RunOnFunction(Function &F) override;
};

#endif // COPY_PROPAGATION_PASS_HPP
