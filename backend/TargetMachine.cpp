
#include "TargetMachine.hpp"
#include <cassert>

bool TargetMachine::SelectInstruction(MachineInstruction *MI) {
  auto Opcode = MI->GetOpcode();

  switch (Opcode) {
  case MachineInstruction::ADD:
    return SelectADD(MI);
  case MachineInstruction::SUB:
    return SelectSUB(MI);
  case MachineInstruction::MUL:
    return SelectMUL(MI);
  case MachineInstruction::DIV:
    return SelectDIV(MI);
  case MachineInstruction::CMP:
    return SelectCMP(MI);
  case MachineInstruction::MOD:
    return SelectMOD(MI);
  case MachineInstruction::SEXT:
    return SelectSEXT(MI);
  case MachineInstruction::TRUNC:
    return SelectTRUNC(MI);
  case MachineInstruction::LOAD_IMM:
    return SelectLOAD_IMM(MI);
  case MachineInstruction::MOV:
    return SelectMOV(MI);
  case MachineInstruction::LOAD:
    return SelectLOAD(MI);
  case MachineInstruction::STORE:
    return SelectSTORE(MI);
  case MachineInstruction::STACK_ADDRESS:
    return SelectSTACK_ADDRESS(MI);
  case MachineInstruction::GLOBAL_ADDRESS:
    return SelectGLOBAL_ADDRESS(MI);
  case MachineInstruction::BRANCH:
    return SelectBRANCH(MI);
  case MachineInstruction::JUMP:
    return SelectJUMP(MI);
  case MachineInstruction::CALL:
    return SelectCALL(MI);
  case MachineInstruction::RET:
    return SelectRET(MI);
  default:
    assert(!"Unimplemented");
  }

  return false;
}