#ifndef MACHINEIRMODULE_HPP
#define MACHINEIRMODULE_HPP

#include "GlobalData.hpp"
#include "MachineFunction.hpp"
#include <vector>

class MachineIRModule {
  using FunctionList = std::vector<MachineFunction>;

public:
  MachineIRModule() {}

  void AddFunction(MachineFunction &F) { Functions.push_back(F); }
  FunctionList &GetFunctions() { return Functions; }
  MachineFunction *GetCurrentFunction() {
    return &Functions[Functions.size() - 1];
  }

  void Print() const {}

private:
  std::vector<GlobalData> GlobalVars;
  FunctionList Functions;
};

#endif
