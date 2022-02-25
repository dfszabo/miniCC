#ifndef REGISTER_CLASS_SELECTION_HPP
#define REGISTER_CLASS_SELECTION_HPP

#include "MachineIRModule.hpp"
#include "TargetMachine.hpp"

class RegisterClassSelection {
public:
  RegisterClassSelection(MachineIRModule *Input, TargetMachine *Target)
      : MIRM(Input), TM(Target) {}

  void Run();

private:
  MachineIRModule *MIRM;
  TargetMachine *TM;
};

#endif
