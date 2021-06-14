#ifndef MODULE_HPP
#define MODULE_HPP

#include "Type.hpp"
#include <cassert>
#include <memory>
#include <vector>

class BasicBlock;
class Function;
class Value;

class Module {
public:
  Module() = default;

  BasicBlock *CurrentBB();

  BasicBlock *GetBB(const size_t Index);

  std::vector<Function>& GetFunctions() { return Functions; }
  std::vector<std::unique_ptr<Value>>& GetGlobalVars() { return GlobalVars; }

  Function *CurrentFunction();

  void AddFunction(Function F);

  void AddGlobalVar(std::unique_ptr<Value> GV);

  bool IsGlobalValue(Value *V) const;

  Value *GetGlobalVar(const std::string &Name) const;

  BasicBlock *CreateBasicBlock();

  void Print() const;

private:
  std::vector<IRType> StructTypes;
  std::vector<std::unique_ptr<Value>> GlobalVars;
  std::vector<Function> Functions;
};

#endif
