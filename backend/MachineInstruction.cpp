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
  case XOR:
    OpcodeStr = "XOR";
    break;
  case LSL:
    OpcodeStr = "LSL";
    break;
  case LSR:
    OpcodeStr = "LSR";
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
  case DIVU:
    OpcodeStr = "DIVU";
    break;
  case MOD:
    OpcodeStr = "MOD";
    break;
  case MODU:
    OpcodeStr = "MODU";
    break;
  case CMP:
    OpcodeStr = "CMP";
    break;
  case ADDF:
    OpcodeStr = "ADDF";
    break;
  case SUBF:
    OpcodeStr = "SUBF";
    break;
  case MULF:
    OpcodeStr = "MULF";
    break;
  case DIVF:
    OpcodeStr = "DIVF";
    break;
  case CMPF:
    OpcodeStr = "CMPF";
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
  case FTOI:
    OpcodeStr = "FTOI";
    break;
  case ITOF:
    OpcodeStr = "ITOF";
    break;
  case ZEXT_LOAD:
    OpcodeStr = "ZEXT_LOAD";
    break;
  case STACK_ADDRESS:
    OpcodeStr = "STACK_ADDRESS";
    break;
  case GLOBAL_ADDRESS:
    OpcodeStr = "GLOBAL_ADDRESS";
    break;
  case LOAD_IMM:
    OpcodeStr = "LOAD_IMM";
    break;
  case MOV:
    OpcodeStr = "MOV";
    break;
  case MOVF:
    OpcodeStr = "MOVF";
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
  case CALL:
    OpcodeStr = "CALL";
    break;
  case RET:
    OpcodeStr = "RET";
    break;
  case INVALID_OP:
    OpcodeStr = "INVALID_OP";
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