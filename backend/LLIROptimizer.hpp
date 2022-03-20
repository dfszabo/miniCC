#ifndef LLIR_OPTIMIZER_HPP
#define LLIR_OPTIMIZER_HPP

class MachineIRModule;
class TargetMachine;

class LLIROptimizer {
public:
  LLIROptimizer(MachineIRModule *Input, TargetMachine *Target)
      : MIRM(Input), TM(Target) {}

  void Run();

private:
  MachineIRModule *MIRM;
  TargetMachine *TM;
};

#endif // LLIR_OPTIMIZER_HPP
