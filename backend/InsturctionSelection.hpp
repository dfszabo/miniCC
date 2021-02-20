#ifndef INSTRUCTION_SELECTION_HPP
#define INSTRUCTION_SELECTION_HPP

#include "../middle_end/IR/Function.hpp"
#include "../middle_end/IR/Module.hpp"
#include "MachineIRModule.hpp"
#include "TargetMachine.hpp"

class InsturctionSelection {
public:
  InsturctionSelection(MachineIRModule *Input, TargetMachine *Target)
      : MIRM(Input), TM(Target) {}

  void InstrSelect();

private:
  MachineIRModule *MIRM;
  TargetMachine *TM;
};

#endif
