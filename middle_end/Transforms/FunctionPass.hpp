#ifndef FUNCTION_PASS_HPP
#define FUNCTION_PASS_HPP

class Function;
class FunctionPass {
public:
  virtual bool RunOnFunction(Function &F) = 0;
};

#endif
