#ifndef MACHINEIRMODULE_HPP
#define MACHINEIRMODULE_HPP

#include "GlobalData.hpp"
#include "MachineFunction.hpp"
#include <vector>

class MachineIRModule {
  using FunctionList = std::vector<MachineFunction>;
  using GlobalsList = std::vector<GlobalData>;

public:
  MachineIRModule() {}

  void AddNewFunction() { Functions.push_back(MachineFunction()); }

  FunctionList &GetFunctions() { return Functions; }
  MachineFunction *GetCurrentFunction() {
    if (Functions.size() == 0)
      return nullptr;
    return &Functions[Functions.size() - 1];
  }

  GlobalsList &GetGlobalDatas() { return GlobalVars; }
  void AddGlobalData(GlobalData &GD) { GlobalVars.push_back(GD); }

  void Print(TargetMachine *TM) const {
    for (auto &F : Functions)
      F.Print(TM);
  }

private:
  GlobalsList GlobalVars;
  FunctionList Functions;
};

#endif
