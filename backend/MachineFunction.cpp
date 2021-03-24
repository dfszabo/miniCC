#include "MachineFunction.hpp"
#include "MachineBasicBlock.hpp"
#include <iostream>

unsigned MachineFunction::GetNextAvailableVReg() {
  // If the next virtual register was computed already once, then just
  // increment it
  if (NextVReg != 0)
    return NextVReg++;

  // If this function was called the first time then here the highest virtual
  // register ID is searched and NextVReg is set to that.
  for (auto &BB : BasicBlocks)
    for (auto &Instr : BB.GetInstructions())
      for (auto &Operand : Instr.GetOperands())
        if (Operand.IsVirtualReg() && Operand.GetReg() > NextVReg)
          NextVReg = Operand.GetReg();

  // The next one is 1 more then the found highest
  return ++NextVReg;
}

void MachineFunction::Print() const {
  std::cout << "function:" << Name << std::endl;
  std::cout << "\tStackFrame:" << std::endl;
  SF.Print();

  for (auto &BB : BasicBlocks)
    BB.Print();
}
