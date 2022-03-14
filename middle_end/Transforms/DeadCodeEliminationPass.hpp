#ifndef DEAD_CODE_ELIMINATION_PASS_HPP
#define DEAD_CODE_ELIMINATION_PASS_HPP

#include "FunctionPass.hpp"

class DeadCodeEliminationPass : public FunctionPass {
public:
  bool RunOnFunction(Function &F) override;
};

#endif // DEAD_CODE_ELIMINATION_PASS_HPP
