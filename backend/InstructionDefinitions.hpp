#ifndef INSTRUCTION_DEFINITIONS_HPP
#define INSTRUCTION_DEFINITIONS_HPP

#include <string>

class TargetInstruction;

class InstructionDefinitions {
public:
  InstructionDefinitions() {}
  virtual ~InstructionDefinitions() {}

  virtual TargetInstruction *GetTargetInstr(unsigned Opcode) { return nullptr; }
  virtual std::string GetInstrString(unsigned index) { return ""; }
};

#endif
