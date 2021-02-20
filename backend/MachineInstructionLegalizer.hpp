#ifndef MACHINE_INSTRUCTION_LEGALIZER_HPP
#define MACHINE_INSTRUCTION_LEGALIZER_HPP

#include "MachineIRModule.hpp"
#include "TargetMachine.hpp"

/// Based on the target machine decide for each yet target independent machine
/// instructions that it is selectable or not.
/// Example: AArch64 (Armv8a) has no native instruction for modulo, therefor MOD
/// is not selectable directly. It must be expanded to a sequence of
/// instructions, which achieve the same result as the expanded operation, but
/// it is legal for the target machine.
class MachineInstructionLegalizer {
public:
  MachineInstructionLegalizer(MachineIRModule *Module, TargetMachine *TM)
      : MIRM(Module), TM(TM) {}

  void Run();

private:
  MachineIRModule *MIRM;
  TargetMachine *TM;
};

#endif
