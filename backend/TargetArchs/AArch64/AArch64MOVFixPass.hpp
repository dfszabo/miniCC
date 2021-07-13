#ifndef AARCH64_MOV_FIX_PASS_HPP
#define AARCH64_MOV_FIX_PASS_HPP

#include "../../MachineIRModule.hpp"
#include "../../TargetMachine.hpp"

class AArch64MOVFixPass {
public:
  AArch64MOVFixPass(MachineIRModule *Module, TargetMachine *TM)
      : MIRM(Module), TM(TM) {}

  void Run();

private:
  MachineIRModule *MIRM;
  TargetMachine *TM;
};

#endif
