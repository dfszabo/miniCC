#include "RISCVInstructionDefinitions.hpp"
#include "../../TargetInstruction.hpp"
#include <cassert>

using namespace RISCV;

RISCVInstructionDefinitions::IRToTargetInstrMap
    RISCVInstructionDefinitions::Instructions = [] {
      IRToTargetInstrMap ret;

      ret[ADD] = {ADD, 32, "addi\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[ADDI] = {ADDI, 32, "addi\t$1, $2, $3", {GPR, GPR, SIMM12}};
      ret[REM] = {REM, 32, "rem\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[LW] = {LW,
                 32,
                 "lw\t$1, $3($2)",
                 {GPR, GPR, SIMM12},
                 TargetInstruction::LOAD};
      ret[SW] = {SW,
                 32,
                 "sw\t$1, $3($2)",
                 {GPR, GPR, SIMM12},
                 TargetInstruction::STORE};
      ret[SLT] = {SLT, 32, "slt\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[SLTI] = {SLTI, 32, "slti\t$1, $2, $3", {GPR, GPR, SIMM12}};
      ret[BEQ] = {BEQ, 32, "beq\t$1, $2, $3", {GPR, GPR, SIMM13_LSB0}};
      ret[BLT] = {BLT, 32, "blt\t$1, $2, $3", {GPR, GPR, SIMM13_LSB0}};
      ret[BNEZ] = {BNEZ, 32, "bnez\t$1, $2", {GPR, SIMM13_LSB0}};
      ret[J] = {J, 32, "j\t$1", {SIMM21_LSB0}};
      ret[RET] = {RET, 32, "ret", {}, TargetInstruction::RETURN};

      return ret;
    }();

TargetInstruction *
RISCVInstructionDefinitions::GetTargetInstr(unsigned Opcode) {
  if (0 == Instructions.count(Opcode))
    return nullptr;

  return &Instructions[Opcode];
}
