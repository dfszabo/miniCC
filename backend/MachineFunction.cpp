#include "MachineFunction.hpp"
#include "MachineBasicBlock.hpp"
#include <iostream>

class TargetMachine;

unsigned MachineFunction::GetNextAvailableVReg() {
  // If this function was called the first time then here the highest virtual
  // register ID is searched and NextVReg is set to that.
  for (auto &[ParamID, ParamLLT] : Parameters)
    if (ParamID == NextVReg)
      NextVReg++;
    else if (ParamID > NextVReg)
      NextVReg = ParamID;

  for (auto &BB : BasicBlocks)
    for (auto &Instr : BB.GetInstructions())
      for (auto &Operand : Instr.GetOperands())
        if (Operand.IsVirtualReg() && Operand.GetReg() >= NextVReg)
          NextVReg =
              Operand.GetReg() == NextVReg ? NextVReg + 1 : Operand.GetReg();

  // The next one is 1 more then the found highest
  return NextVReg++;
}

void MachineFunction::Print(TargetMachine *TM) const {
  std::cout << "function:" << Name << std::endl;
  std::cout << "\tStackFrame:" << std::endl;
  SF.Print();

  for (auto &BB : BasicBlocks)
    BB.Print(TM);
}
