#include "RISCVInstructionDefinitions.hpp"

using namespace RISCV;

RISCVInstructionDefinitions::RISCVInstructionDefinitions() {
  InstrEnumStrings = {
      "LB",    "LH",     "LW",    "LBU", "LHU",  "SB",  "SH",   "SW",   "SLL",
      "SLLI",  "SRL",    "SRLI",  "SRA", "SRAI", "ADD", "ADDI", "SUB",  "LUI",
      "AUIPC", "XOR",    "XORI",  "OR",  "ORI",  "AND", "ANDI", "SLT",  "SLTI",
      "SLTU",  "SLTIU",  "BEQ",   "BNE", "BLT",  "BGE", "BLTU", "BGEU", "MUL",
      "MULH",  "MULHSU", "MULHU", "DIV", "DIVU", "REM", "REMU", "NOT",  "MV",
      "SEQZ",  "SNEZ",   "BNEZ",  "J",   "CALL", "RET", "LI",
  };
}

RISCVInstructionDefinitions::IRToTargetInstrMap
    RISCVInstructionDefinitions::Instructions = [] {
      IRToTargetInstrMap ret;

      // Loads
      ret[LB] = {LB,
                 32,
                 "lb\t$1, $3($2)",
                 {GPR, GPR, SIMM12},
                 TargetInstruction::LOAD};
      ret[LH] = {LH,
                 32,
                 "lh\t$1, $3($2)",
                 {GPR, GPR, SIMM12},
                 TargetInstruction::LOAD};
      ret[LW] = {LW,
                 32,
                 "lw\t$1, $3($2)",
                 {GPR, GPR, SIMM12},
                 TargetInstruction::LOAD};
      ret[LBU] = {LBU,
                  32,
                  "lbu\t$1, $3($2)",
                  {GPR, GPR, SIMM12},
                  TargetInstruction::LOAD};
      ret[LHU] = {LHU,
                  32,
                  "lhu\t$1, $3($2)",
                  {GPR, GPR, SIMM12},
                  TargetInstruction::LOAD};

      // Stores
      ret[SB] = {SB,
                 32,
                 "sb\t$1, $3($2)",
                 {GPR, GPR, SIMM12},
                 TargetInstruction::STORE};
      ret[SH] = {SH,
                 32,
                 "sh\t$1, $3($2)",
                 {GPR, GPR, SIMM12},
                 TargetInstruction::STORE};
      ret[SW] = {SW,
                 32,
                 "sw\t$1, $3($2)",
                 {GPR, GPR, SIMM12},
                 TargetInstruction::STORE};

      // Shifts
      ret[SLL] = {SLL, 32, "sll\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[SLLI] = {SLLI, 32, "slli\t$1, $2, $3", {GPR, GPR, SIMM12}};
      ret[SRL] = {SLL, 32, "srl\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[SRLI] = {SLLI, 32, "srli\t$1, $2, $3", {GPR, GPR, SIMM12}};
      ret[SRA] = {SRA, 32, "sra\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[SRAI] = {SRAI, 32, "srai\t$1, $2, $3", {GPR, GPR, SIMM12}};

      // Arithmetic
      ret[ADD] = {ADD, 32, "add\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[ADDI] = {ADDI, 32, "addi\t$1, $2, $3", {GPR, GPR, SIMM12}};
      ret[SUB] = {SUB, 32, "sub\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[LUI] = {LUI, 32, "lui\t$1, $2", {GPR, SIMM12}};
      ret[AUIPC] = {AUIPC, 32, "auipc\t$1, $2", {GPR, UIMM20}};

      // Logical
      ret[XOR] = {XOR, 32, "xor\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[XORI] = {XORI, 32, "xori\t$1, $2, $3", {GPR, GPR, SIMM12}};
      ret[OR] = {OR, 32, "or\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[ORI] = {ORI, 32, "ori\t$1, $2, $3", {GPR, GPR, SIMM12}};
      ret[AND] = {AND, 32, "and\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[ANDI] = {ANDI, 32, "andi\t$1, $2, $3", {GPR, GPR, UIMM12}};

      // Compare
      ret[SLT] = {SLT, 32, "slt\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[SLTI] = {SLTI, 32, "slti\t$1, $2, $3", {GPR, GPR, SIMM12}};
      ret[SLTU] = {SLTU, 32, "sltu\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[SLTIU] = {SLTIU, 32, "sltiu\t$1, $2, $3", {GPR, GPR, UIMM12}};

      // Branches
      ret[BEQ] = {BEQ, 32, "beq\t$1, $2, $3", {GPR, GPR, SIMM13_LSB0}};
      ret[BNE] = {BNE, 32, "bne\t$1, $2, $3", {GPR, GPR, SIMM13_LSB0}};
      ret[BLT] = {BLT, 32, "blt\t$1, $2, $3", {GPR, GPR, SIMM13_LSB0}};
      ret[BGE] = {BGE, 32, "bge\t$1, $2, $3", {GPR, GPR, SIMM13_LSB0}};
      ret[BLTU] = {BLTU, 32, "bltu\t$1, $2, $3", {GPR, GPR, SIMM13_LSB0}};
      ret[BGEU] = {BGEU, 32, "bgeu\t$1, $2, $3", {GPR, GPR, SIMM13_LSB0}};

      // M extension
      ret[MUL] = {MUL, 32, "mul\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[MULH] = {MULH, 32, "mulh\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[MULHSU] = {MULHSU, 32, "mulhsu\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[MULHU] = {MULHU, 32, "mulhu\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[DIV] = {DIV, 32, "div\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[DIVU] = {DIVU, 32, "divu\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[REM] = {REM, 32, "rem\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[REMU] = {REMU, 32, "remu\t$1, $2, $3", {GPR, GPR, GPR}};

      // Pseudo instructions
      ret[NOT] = {NOT, 32, "not\t$1, $2", {GPR, GPR}};
      ret[MV] = {MV, 32, "mv\t$1, $2", {GPR, GPR}};
      ret[SEQZ] = {SEQZ, 32, "seqz\t$1, $2", {GPR, GPR}};
      ret[SNEZ] = {SNEZ, 32, "snez\t$1, $2", {GPR, GPR}};
      ret[BNEZ] = {BNEZ, 32, "bnez\t$1, $2", {GPR, SIMM13_LSB0}};
      ret[J] = {J, 32, "j\t$1", {SIMM21_LSB0}};
      ret[CALL] = {CALL, 32, "call\t$1", {UIMM32}};
      ret[RET] = {RET, 32, "ret", {}, TargetInstruction::RETURN};
      ret[LI] = {LI, 32, "li\t$1, $2", {GPR, SIMM13_LSB0}};

      return ret;
    }();

TargetInstruction *
RISCVInstructionDefinitions::GetTargetInstr(unsigned Opcode) {
  if (0 == Instructions.count(Opcode))
    return nullptr;

  return &Instructions[Opcode];
}
