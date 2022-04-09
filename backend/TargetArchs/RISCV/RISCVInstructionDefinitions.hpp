#ifndef RISCV_INSTRUCTION_DEFINITIONS_HPP
#define RISCV_INSTRUCTION_DEFINITIONS_HPP

#include "../../InstructionDefinitions.hpp"
#include "../../MachineInstruction.hpp"
#include "../../TargetInstruction.hpp"
#include <map>

namespace RISCV {

enum Opcodes : unsigned {
  LB,
  LH,
  LW,
  LBU,
  LHU,
  SB,
  SH,
  SW,
  SLL,
  SLLI,
  SRL,
  SRLI,
  SRA,
  SRAI,
  ADD,
  ADDI,
  SUB,
  LUI,
  AUIPC,
  XOR,
  XORI,
  OR,
  ORI,
  AND,
  ANDI,
  SLT,
  SLTI,
  SLTU,
  SLTIU,
  BEQ,
  BNE,
  BLT,
  BGE,
  BLTU,
  BGEU,
  MUL,
  MULH,
  MULHSU,
  MULHU,
  DIV,
  DIVU,
  REM,
  REMU,
  NOT,
  MV,
  SEQZ,
  SNEZ,
  BNEZ,
  J,
  CALL,
  RET,
  LI,
};

enum OperandTypes : unsigned {
  GPR,
  GPR32,
  SIMM12,
  UIMM12,
  SIMM13_LSB0,
  UIMM20,
  SIMM21_LSB0,
  UIMM32,
};

class RISCVInstructionDefinitions : public InstructionDefinitions {
  using IRToTargetInstrMap = std::map<unsigned, TargetInstruction>;

public:
  RISCVInstructionDefinitions();
  ~RISCVInstructionDefinitions() override {}
  TargetInstruction *GetTargetInstr(unsigned Opcode) override;
  std::string GetInstrString(unsigned index) override {
    assert(index < InstrEnumStrings.size() && "Out of bound access");
    return InstrEnumStrings[index];
  }

private:
  static IRToTargetInstrMap Instructions;
  std::vector<std::string> InstrEnumStrings;
};

} // namespace RISCV

#endif
