#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "Type.hpp"
#include <memory>
#include <string>
#include <vector>

class BasicBlock;
class FunctionParameter;

class Function {
public:
  Function(const std::string &Name, IRType RT);

  /// Since unique_ptr is not copyable, therefore this class should not as well
  Function(const Function &) = delete;
  Function(Function &&) = default;

  BasicBlock *GetCurrentBB();
  BasicBlock *GetBB(const size_t Index);

  void CreateBasicBlock();

  void Insert(std::unique_ptr<BasicBlock> BB);
  void Insert(std::unique_ptr<FunctionParameter> FP);

  void Print() const;

private:
  std::string Name;
  IRType ReturnType;
  std::vector<std::unique_ptr<FunctionParameter>> Parameters;
  std::vector<std::unique_ptr<BasicBlock>> BasicBlocks;
};

#endif
