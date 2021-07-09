#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "Type.hpp"
#include <memory>
#include <string>
#include <vector>

class BasicBlock;
class FunctionParameter;

class Function {
  using BasicBlockList = std::vector<std::unique_ptr<BasicBlock>>;
  using ParameterList = std::vector<std::unique_ptr<FunctionParameter>>;

public:
  Function(const std::string &Name, IRType RT);

  /// Since unique_ptr is not copyable, therefore this class should not as well
  Function(const Function &) = delete;
  Function(Function &&) = default;

  BasicBlock *GetCurrentBB();
  BasicBlock *GetBB(const size_t Index);

  std::string &GetName() { return Name; }

  BasicBlockList &GetBasicBlocks() { return BasicBlocks; }

  ParameterList &GetParameters() { return Parameters; }

  std::string &GetIgnorableStructVarName() { return IgnorableStructVarName; }
  void SetIgnorableStructVarName(std::string &Name) {
    IgnorableStructVarName = Name;
  }

  bool IsRetTypeVoid() { return ReturnType.IsVoid(); }

  void CreateBasicBlock();

  void Insert(std::unique_ptr<BasicBlock> BB);
  void Insert(std::unique_ptr<FunctionParameter> FP);

  void Print() const;

private:
  std::string Name;
  IRType ReturnType;
  ParameterList Parameters;
  BasicBlockList BasicBlocks;

  std::string IgnorableStructVarName = "";
};

#endif
