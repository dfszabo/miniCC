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
  MachineInstruction CreateLOADInstruction(unsigned Source, int64_t Offset);

  void InsertStackAdjustmentUpward(MachineFunction &Func);
  void InsertStackAdjustmentDownward(MachineFunction &Func);

private:
  MachineIRModule *MIRM;
  TargetMachine *TM;
};

#endif