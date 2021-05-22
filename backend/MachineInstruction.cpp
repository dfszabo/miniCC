#include "MachineInstruction.hpp"
#include "TargetMachine.hpp"

void MachineInstruction::Print(TargetMachine *TM) const {
  std::string OpcodeStr;

  switch (Opcode) {
  case AND:
    OpcodeStr = "AND";
    break;
  case OR:
    OpcodeStr = "OR";
    break;
  case ADD:
    OpcodeStr = "ADD";
    break;
  case SUB:
    OpcodeStr = "SUB";
    break;
  case MUL:
    OpcodeStr = "MUL";
    break;
  case DIV:
    OpcodeStr = "DIV";
    break;
  case MOD:
    OpcodeStr = "MOD";
    break;
  case CMP:
    OpcodeStr = "CMP";
    break;
  case SEXT:
    OpcodeStr = "SEXT";
    break;
  case ZEXT:
    OpcodeStr = "ZEXT";
    break;
  case TRUNC:
    OpcodeStr = "TRUNC";
    break;
  case STACK_ADDRESS:
    OpcodeStr = "STACK_ADDRESS";
    break;
  case LOAD_IMM:
    OpcodeStr = "LOAD_IMM";
    break;
  case STORE:
    OpcodeStr = "STORE";
    break;
  case LOAD:
    OpcodeStr = "LOAD";
    break;
  case JUMP:
    OpcodeStr = "JUMP";
    break;
  case BRANCH:
    OpcodeStr = "BRANCH";
    break;
  case RET:
    OpcodeStr = "RET";
    break;
  default:
    OpcodeStr = TM->GetInstrDefs()->GetInstrString(Opcode);
    break;
  }

  std::string Spaces = std::string( 16 - OpcodeStr.length(), ' ' );
  std::cout << OpcodeStr << Spaces;
  for (size_t i = 0; i < Operands.size(); i++) {
    Operands[i].Print(TM);
    if (i < Operands.size() - 1)
      std::cout << ", ";
  }
  std::cout << std::endl;
}