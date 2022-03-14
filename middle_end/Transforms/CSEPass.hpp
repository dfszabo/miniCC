#ifndef CSE_PASS_HPP
#define CSE_PASS_HPP

#include "FunctionPass.hpp"

class CSEPass : public FunctionPass {
public:
  bool RunOnFunction(Function &F) override;
};

#endif // CSE_PASS_HPP
