#ifndef INSTRUCTION_DEFINITIONS_HPP
#define INSTRUCTION_DEFINITIONS_HPP

class TargetInstruction;

class InstructionDefinitions {
public:
  InstructionDefinitions() {}
  virtual ~InstructionDefinitions() {}

  virtual TargetInstruction *GetTargetInstr(unsigned Opcode) { return nullptr; }
};

#endif
