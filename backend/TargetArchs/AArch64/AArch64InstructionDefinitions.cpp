#include "AArch64InstructionDefinitions.hpp"

using namespace AArch64;

AArch64InstructionDefinitions::AArch64InstructionDefinitions() {
  InstrEnumStrings = {
      "ADD_rrr",  "ADD_rri",  "AND_rrr",  "AND_rri",   "ORR_rrr", "ORR_rri",
      "EOR_rrr",  "EOR_rri",  "LSL_rrr",  "LSL_rri",   "LSR_rrr", "LSR_rri",
      "SUB_rrr",  "SUB_rri",  "SUBS",     "MUL_rri",   "MUL_rrr", "SDIV_rri",
      "SDIV_rrr", "UDIV_rrr", "CMP_ri",   "CMP_rr",    "CSET_eq", "CSET_ne",
      "CSET_lt",  "CSET_gt",  "SXTB",     "SXTH",      "SXTW",    "UXTB",
      "UXTH",     "UXTW",     "MOV_rc",   "MOV_rr",    "MOVK_ri", "MVN_rr",
      "FADD_rrr", "FSUB_rrr", "FDIV_rrr", "FMUL_rrr",  "FMOV_rr", "FMOV_ri",
      "FCMP_rr",  "FCMP_ri",  "SCVTF_rr", "FCVTZS_rr", "ADRP",    "LDR",
      "LDRB",     "LDRH",     "STR",      "STRB",      "STRH",    "BEQ",
      "BNE",      "BGE",      "BGT",      "BLE",       "BLT",     "B",
      "BL",       "RET"};
}

AArch64InstructionDefinitions::IRToTargetInstrMap
    AArch64InstructionDefinitions::Instructions = [] {
      IRToTargetInstrMap ret;

      ret[ADD_rrr] = {ADD_rrr, 32, "add\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[ADD_rri] = {ADD_rri, 32, "add\t$1, $2, #$3", {GPR, GPR, UIMM12}};
      ret[AND_rrr] = {ADD_rrr, 32, "and\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[AND_rri] = {ADD_rri, 32, "and\t$1, $2, #$3", {GPR, GPR, UIMM12}};
      ret[ORR_rrr] = {ORR_rrr, 32, "orr\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[ORR_rri] = {ORR_rri, 32, "orr\t$1, $2, #$3", {GPR, GPR, UIMM12}};
      ret[EOR_rrr] = {EOR_rrr, 32, "eor\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[EOR_rri] = {EOR_rri, 32, "eor\t$1, $2, #$3", {GPR, GPR, UIMM12}};
      ret[LSL_rrr] = {LSL_rrr, 32, "lsl\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[LSL_rri] = {LSL_rri, 32, "lsl\t$1, $2, #$3", {GPR, GPR, UIMM12}};
      ret[LSR_rrr] = {LSR_rrr, 32, "lsr\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[LSR_rri] = {LSR_rri, 32, "lsr\t$1, $2, #$3", {GPR, GPR, UIMM12}};
      ret[SUB_rrr] = {SUB_rrr, 32, "sub\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[SUB_rri] = {SUB_rri, 32, "sub\t$1, $2, #$3", {GPR, GPR, UIMM12}};
      ret[SUBS] = {SUBS, 32, "subs\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[SDIV_rrr] = {SDIV_rrr, 32, "sdiv\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[SDIV_rri] = {SDIV_rri, 32, "sdiv\t$1, $2, #$3", {GPR, GPR, UIMM12}};
      ret[UDIV_rrr] = {UDIV_rrr, 32, "udiv\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[MUL_rrr] = {MUL_rri, 32, "mul\t$1, $2, $3", {GPR, GPR, GPR}};
      ret[MUL_rri] = {MUL_rrr, 32, "mul\t$1, $2, #$3", {GPR, GPR, UIMM12}};
      ret[CMP_rr] = {CMP_rr, 32, "cmp\t$1, $2", {GPR, GPR}};
      ret[CMP_ri] = {CMP_ri, 32, "cmp\t$1, #$2", {GPR, UIMM12}};
      ret[CSET_eq] = {CSET_eq, 32, "cset\t$1, eq", {GPR}};
      ret[CSET_ne] = {CSET_ne, 32, "cset\t$1, ne", {GPR}};
      ret[CSET_lt] = {CSET_lt, 32, "cset\t$1, lt", {GPR}};
      ret[CSET_gt] = {CSET_gt, 32, "cset\t$1, gt", {GPR}};
      ret[SXTB] = {SXTB, 32, "sxtb\t$1, $2", {GPR, GPR}};
      ret[SXTH] = {SXTH, 32, "sxth\t$1, $2", {GPR, GPR}};
      ret[SXTW] = {SXTW, 32, "sxtw\t$1, $2", {GPR, GPR}};
      ret[UXTB] = {UXTB, 32, "uxtb\t$1, $2", {GPR, GPR}};
      ret[UXTH] = {UXTH, 32, "uxth\t$1, $2", {GPR, GPR}};
      ret[UXTW] = {UXTW, 32, "uxtw\t$1, $2", {GPR, GPR}};
      ret[MOV_rc] = {MOV_rc, 32, "mov\t$1, #$2", {GPR, UIMM16}};
      ret[MOV_rr] = {MOV_rr, 32, "mov\t$1, $2", {GPR, GPR}};
      ret[MOVK_ri] = {MOVK_ri, 32, "movk\t$1, #$2, lsl #$3", {GPR, GPR, UIMM4}};
      ret[MVN_rr] = {MVN_rr, 32, "mvn\t$1, $2", {GPR, GPR}};

      // Floating point instructions
      ret[FADD_rrr] = {FADD_rrr, 32, "fadd\t$1, $2, $3", {FPR, FPR, FPR}};
      ret[FSUB_rrr] = {FSUB_rrr, 32, "fsub\t$1, $2, $3", {FPR, FPR, FPR}};
      ret[FMUL_rrr] = {FMUL_rrr, 32, "fmul\t$1, $2, $3", {FPR, FPR, FPR}};
      ret[FDIV_rrr] = {FDIV_rrr, 32, "fdiv\t$1, $2, $3", {FPR, FPR, FPR}};
      ret[FMOV_rr] = {FMOV_rr, 32, "fmov\t$1, $2", {FPR, GPR}};
      ret[FMOV_ri] = {FMOV_ri, 32, "fmov\t$1, #$2", {FPR, UIMM16}};
      ret[FCMP_rr] = {FCMP_rr, 32, "fcmp\t$1, $2", {FPR, GPR}};
      ret[FCMP_ri] = {FCMP_ri, 32, "fcmp\t$1, #$2", {FPR, UIMM12}};
      ret[SCVTF_rr] = {SCVTF_rr, 32, "scvtf\t$1, $2", {FPR, GPR}};
      ret[FCVTZS_rr] = {FCVTZS_rr, 32, "fcvtzs\t$1, $2", {GPR, FPR}};

      ret[ADRP] = {ADRP, 32, "adrp\t$1, $2", {GPR, GPR}};
      ret[LDR] = {LDR,
                  32,
                  "ldr\t$1, [$2, #$3]",
                  {GPR, GPR, SIMM12},
                  TargetInstruction::LOAD};
      ret[LDRB] = {LDRB,
                   32,
                   "ldrb\t$1, [$2, #$3]",
                   {GPR, GPR, SIMM12},
                   TargetInstruction::LOAD};
      ret[LDRH] = {LDRH,
                   32,
                   "ldrh\t$1, [$2, #$3]",
                   {GPR, GPR, SIMM12},
                   TargetInstruction::LOAD};
      ret[STR] = {STR,
                  32,
                  "str\t$1, [$2, #$3]",
                  {GPR, GPR, SIMM12},
                  TargetInstruction::STORE};
      ret[STRB] = {STRB,
                   32,
                   "strb\t$1, [$2, #$3]",
                   {GPR, GPR, SIMM12},
                   TargetInstruction::STORE};
      ret[STRH] = {STRH,
                   32,
                   "strh\t$1, [$2, #$3]",
                   {GPR, GPR, SIMM12},
                   TargetInstruction::STORE};
      ret[BGE] = {BGE, 32, "b.ge\t$1", {SIMM21_LSB0}};
      ret[BGT] = {BGT, 32, "b.gt\t$1", {SIMM21_LSB0}};
      ret[BLE] = {BLE, 32, "b.le\t$1", {SIMM21_LSB0}};
      ret[BLT] = {BLT, 32, "b.lt\t$1", {SIMM21_LSB0}};
      ret[BEQ] = {BEQ, 32, "b.eq\t$1", {SIMM21_LSB0}};
      ret[BNE] = {BNE, 32, "b.ne\t$1", {SIMM21_LSB0}};
      ret[B] = {B, 32, "b\t$1", {SIMM21_LSB0}};
      ret[BL] = {BL, 32, "bl\t$1", {SIMM21_LSB0}};
      ret[RET] = {RET, 32, "ret", {}, TargetInstruction::RETURN};

      return ret;
    }();

TargetInstruction *
AArch64InstructionDefinitions::GetTargetInstr(unsigned Opcode) {
  if (0 == Instructions.count(Opcode))
    return nullptr;

  return &Instructions[Opcode];
}
