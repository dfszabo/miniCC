#ifndef IRTOLLIR_HPP
#define IRTOLLIR_HPP

#include "../middle_end/IR/Module.hpp"
#include "MachineIRModule.hpp"

/// This class responsible to translate the middle end's IR to a lower level IR
/// which used in the backend.
class IRtoLLIR {
public:
  IRtoLLIR(Module &IRModule, MachineIRModule *TranslUnit)
      : IRM(IRModule), TU(TranslUnit) {}

  void GenerateLLIRFromIR();

  MachineIRModule *GetMachIRMod() { return TU; }

private:
  Module &IRM;
  MachineIRModule *TU;
};

#endif
