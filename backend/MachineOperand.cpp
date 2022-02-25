#include "MachineOperand.hpp"
#include "TargetMachine.hpp"


void MachineOperand::Print(TargetMachine *TM) const {
  switch (Type) {
  case REGISTER:
    if (Virtual)
      std::cout << "%vreg" << IntVal;
    else if (!Virtual && TM) {
      TargetRegister *Reg =
          TM->GetRegInfo()->GetRegisterByID(GetReg());
      std::string RegStr;
      if (Reg->GetAlias() != "")
        RegStr = Reg->GetAlias();
      else
        RegStr = Reg->GetName();

      std::cout << RegStr;
      return;
    }
    else
      std::cout << "%" << IntVal;
    break;
  case MEMORY_ADDRESS:
    std::cout << "%ptr_vreg" << IntVal;
    if (Offset > 0)
      std::cout << "+" << Offset;
    break;
  case INT_IMMEDIATE:
    std::cout << (int64_t)IntVal;
    break;
  case FP_IMMEDIATE:
    std::cout << std::to_string(FloatVal);
    break;
  case STACK_ACCESS:
    std::cout << "stack" << IntVal << "+" << Offset;
    break;
  case PARAMETER:
    std::cout << "@" << IntVal;
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

  // Attach size information if the operand has an LLT type, also in case
  // of virtual registers and virtual ptr registers attach register class
  // information as well
  if (LLT.IsValid()) {
    std::string str;
    str = "(" + LLT.ToString();
    if ((Type == REGISTER || Type == MEMORY_ADDRESS) && IsVirtual())
      str += RegisterClass == ~0u
                 ? ":_"
                 : ":" + TM->GetRegInfo()->GetRegClassString(RegisterClass);
    str += ")";
    std::cout << str;
  }
}