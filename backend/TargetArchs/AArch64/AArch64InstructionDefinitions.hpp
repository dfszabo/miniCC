#ifndef AARCH64_INSTRUCTION_DEFINITIONS_HPP
#define AARCH64_INSTRUCTION_DEFINITIONS_HPP

#include "../../InstructionDefinitions.hpp"
#include "../../MachineInstruction.hpp"
#include "../../TargetInstruction.hpp"
#include <map>

namespace AArch64 {

enum Opcodes : unsigned {
  ADD_rrr,
  ADD_rri,
  SUB_rrr,
  SUB_rri,
  SUBS,
  MUL_rri,
  MUL_rrr,
  SDIV_rri,
  SDIV_rrr,
  CMP_ri,
  CMP_rr,
  CSET,
  MOV_rc,
  LDR,
  STR,
  BEQ,
  BNE,
  BGE,
  BGT,
  BLE,
  BLT,
  B,
  RET,
};

enum OperandTypes : unsigned {
  GPR,
  SIMM12,
  UIMM12,
  UIMM16,
  SIMM13_LSB0,
  SIMM21_LSB0,
};

class AArch64InstructionDefinitions : public InstructionDefinitions {
  using IRToTargetInstrMap = std::map<unsigned, TargetInstruction>;

public:
  AArch64InstructionDefinitions() {}
  ~AArch64InstructionDefinitions() override {}
  TargetInstruction *GetTargetInstr(unsigned Opcode) override;

private:
  static IRToTargetInstrMap Instructions;
};

} // namespace AArch64

#endif
