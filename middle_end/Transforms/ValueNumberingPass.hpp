#ifndef VALUE_NUMBERING_PASS_HPP
#define VALUE_NUMBERING_PASS_HPP

#include "FunctionPass.hpp"

class ValueNumberingPass : public FunctionPass {
public:
  virtual bool RunOnFunction(Function &F) override;
};

#endif // VALUE_NUMBERING_PASS_HPP
