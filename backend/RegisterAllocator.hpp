#ifndef REGISTER_ALLOCATOR_HPP
#define REGISTER_ALLOCATOR_HPP

#include "MachineIRModule.hpp"
#include "TargetMachine.hpp"

class RegisterAllocator {
public:
  RegisterAllocator(MachineIRModule *Module, TargetMachine *TM)
      : MIRM(Module), TM(TM) {}

  void RunRA();

private:
  MachineIRModule *MIRM;
  TargetMachine *TM;
};

#endif
