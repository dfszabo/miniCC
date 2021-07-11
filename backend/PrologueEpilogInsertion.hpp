#ifndef PROLOGUE_EPILOG_INSERTION_HPP
#define PROLOGUE_EPILOG_INSERTION_HPP

#include "MachineIRModule.hpp"
#include "TargetMachine.hpp"

class PrologueEpilogInsertion {
public:
  PrologueEpilogInsertion(MachineIRModule *Module, TargetMachine *TM)
      : MIRM(Module), TM(TM) {}

  void Run();

  MachineInstruction CreateADDInstruction(int64_t StackAdjustmentSize);

  void InsertLinkRegisterSave(MachineFunction &Func);
  void InsertLinkRegisterReload(MachineFunction &Func);
  void InsertStackAdjustmentUpward(MachineFunction &Func);
  void InsertStackAdjustmentDownward(MachineFunction &Func);
  // TODO: naming inconsistency...
  void SpillClobberedCalleeSavedRegisters(MachineFunction &Func);
  void ReloadClobberedCalleeSavedRegisters(MachineFunction &Func);

private:
  MachineInstruction CreateSTORE(MachineFunction &Func, unsigned Register);
  MachineInstruction CreateLOAD(MachineFunction &Func, unsigned Register);

  MachineIRModule *MIRM;
  TargetMachine *TM;
};

#endif