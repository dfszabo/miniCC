#ifndef IRTOLLIR_HPP
#define IRTOLLIR_HPP

#include "../middle_end/IR/Module.hpp"
#include "MachineIRModule.hpp"
#include "TargetMachine.hpp"
#include "../middle_end/IR/Instructions.hpp"

/// This class responsible to translate the middle end's IR to a lower level IR
/// which used in the backend.
class IRtoLLIR {
public:
  IRtoLLIR(Module &IRModule, MachineIRModule *TranslUnit, TargetMachine *TM)
      : IRM(IRModule), TU(TranslUnit), TM(TM) {}

  void GenerateLLIRFromIR();

  MachineIRModule *GetMachIRMod() { return TU; }

private:
  void HandleFunctionParams(Function &F, MachineFunction *Func);
  MachineInstruction ConvertToMachineInstr(Instruction *Instr,
                                          MachineBasicBlock *BB,
                                          std::vector<MachineBasicBlock> &BBs);
  Module &IRM;
  MachineIRModule *TU;
  TargetMachine *TM;

  std::map<std::string, std::vector<unsigned>> StructToRegMap;
};

#endif
