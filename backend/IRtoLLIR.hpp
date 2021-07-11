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

  MachineOperand GetMachineOperandFromValue(Value *Val, MachineBasicBlock *MBB);

  void GenerateLLIRFromIR();

  MachineIRModule *GetMachIRMod() { return TU; }

  void Reset() {
    StructToRegMap.clear();
    StructByIDToRegMap.clear();
    IRVregToLLIRVreg.clear();
  }

private:
  void HandleFunctionParams(Function &F, MachineFunction *Func);
  MachineInstruction ConvertToMachineInstr(Instruction *Instr,
                                          MachineBasicBlock *BB,
                                          std::vector<MachineBasicBlock> &BBs);
  Module &IRM;
  MachineIRModule *TU;
  TargetMachine *TM;

  std::map<std::string, std::vector<unsigned>> StructToRegMap;

  /// to keep track in which registers the struct is currently living
  std::map<unsigned, std::vector<unsigned>> StructByIDToRegMap;

  /// Keep track what IR virtual registers were mapped to what LLIR virtual
  /// registers. This needed since while translating from IR to LLIR occasionally
  /// new instructions are added with possible new virtual registers.
  std::map<unsigned, unsigned> IRVregToLLIRVreg;
};

#endif
