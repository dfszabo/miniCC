#include "MachineOperand.hpp"
#include "TargetMachine.hpp"


void MachineOperand::Print(TargetMachine *TM) const {
  switch (Type) {
  case REGISTER:
    if (Virtual)
      std::cout << "%vreg" << Value;
    else if (!Virtual && TM) {
      TargetRegister *Reg =
          TM->GetRegInfo()->GetRegisterByID(GetReg());
      std::string RegStr;
      if (Reg->GetAlias() != "")
        RegStr = Reg->GetAlias();
      else
        RegStr = Reg->GetName();

      std::cout << RegStr;
    }
    else
      std::cout << "%" << Value;
    break;
  case MEMORY_ADDRESS:
    std::cout << "%ptr_vreg" << Value;
    if (Offset > 0)
      std::cout << "+" << Offset;
    break;
  case INT_IMMEDIATE:
    std::cout << (int64_t)Value;
    break;
  case STACK_ACCESS:
    std::cout << "stack" << Value << "+" << Offset;
    break;
  case PARAMETER:
    std::cout << "@" << Value;
    break;
  case LABEL:
    std::cout << "<" << Label << ">";
    break;
  case FUNCTION_NAME:
    std::cout << "@" << Label;
    break;
  case GLOBAL_SYMBOL:
    std::cout << "@" << GlobalSymbol;
    break;
  default:
    break;
  }

  if (LLT.IsValid())
    std::cout << "(" << LLT.ToString() << ")";
}