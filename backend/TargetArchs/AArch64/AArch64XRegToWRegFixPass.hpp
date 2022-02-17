#ifndef AARCH64_XREG_TO_WREG_FIX_PASS_HPP
#define AARCH64_XREG_TO_WREG_FIX_PASS_HPP

#include "../../MachineIRModule.hpp"
#include "../../TargetMachine.hpp"

/// A pass to fix the problem of differently sized register operands.
/// Namely the case when: op Wx, Xy ...
/// In which the Xy register should be changed to Wy.
class AArch64XRegToWRegFixPass {
public:
  AArch64XRegToWRegFixPass(MachineIRModule *Module, TargetMachine *TM)
      : MIRM(Module), TM(TM) {}

  void Run();

private:
  MachineIRModule *MIRM;
  TargetMachine *TM;
};

#endif
