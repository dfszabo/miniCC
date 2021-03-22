#ifndef MACHINEFUNCTION_HPP
#define MACHINEFUNCTION_HPP

#include "LowLevelType.hpp"
#include "MachineBasicBlock.hpp"
#include "StackFrame.hpp"
#include <vector>

class MachineFunction {
  using BasicBlockList = std::vector<MachineBasicBlock>;
  using ParamList = std::vector<std::pair<unsigned, LowLevelType>>;

public:
  MachineFunction() {}
  MachineFunction(BasicBlockList BBs) : BasicBlocks(BBs) {}

  void SetBasicBlocks(BasicBlockList BBs) { BasicBlocks = BBs; }
  BasicBlockList &GetBasicBlocks() { return BasicBlocks; }

  std::string &GetName() { return Name; }
  void SetName(std::string &Name) { this->Name = Name; }

  void InsertStackSlot(unsigned ID, unsigned Size) {
    SF.InsertStackSlot(ID, Size);
  }

  void InsertParameter(unsigned ID, LowLevelType LLT) {
    Parameters.push_back({ID, LLT});
  }

  ParamList GetParameters() { return Parameters; }

  unsigned GetStackFrameSize() { return SF.GetSize(); }
  unsigned GetStackObjectPosition(unsigned ID) { return SF.GetPosition(ID); }

  bool IsStackSlot(unsigned ID) { return SF.IsStackSlot(ID); }

  /// Get the next available virtual register.
  unsigned GetNextAvailableVReg();

  void Print() const;

private:
  std::string Name;
  ParamList Parameters;
  StackFrame SF;
  BasicBlockList BasicBlocks;
  unsigned NextVReg = 0;
};

#endif
