#include "Instructions.hpp"
#include "BasicBlock.hpp"

std::string Instruction::AsString(IKind IK) {
  switch (IK) {
  case AND:
    return "and";
  case OR:
    return "or";
  case XOR:
    return "xor";
  case LSL:
    return "lsl";
  case LSR:
    return "lsr";
  case ADD:
    return "add";
  case SUB:
    return "sub";
  case MUL:
    return "mul";
  case DIV:
    return "div";
  case DIVU:
    return "divu";
  case MOD:
    return "mod";
  case MODU:
    return "modu";
  case SEXT:
    return "sext";
  case ZEXT:
    return "zext";
  case TRUNC:
    return "trunc";
  case FTOI:
    return "ftoi";
  case ITOF:
    return "itof";
  case CALL:
    return "call";
  case JUMP:
    return "j";
  case BRANCH:
    return "br";
  case RET:
    return "ret";
  case LOAD:
    return "ld";
  case STORE:
    return "str";
  case STACK_ALLOC:
    return "sa";
  case GET_ELEM_PTR:
    return "gep";
  case MEM_COPY:
    return "memcopy";
  case CMP:
    return "cmp";
  case MOV:
    return "mov";
  default:
    assert(!"Unknown instruction kind.");
    break;
  }
}

void BinaryInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  std::cout << ValueString() << ", ";
  std::cout << LHS->ValueString() << ", ";
  std::cout << RHS->ValueString() << std::endl;
}

void UnaryInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  std::cout << ValueString() << ", ";
  std::cout << Op->ValueString() << std::endl;
}

const char *CompareInstruction::GetRelString() const {
  switch (Relation) {
  case EQ:
    return "eq";
  case NE:
    return "ne";
  case LT:
    return "lt";
  case GT:
    return "gt";
  case LE:
    return "le";
  case GE:
    return "ge";
  default:
    assert(!"Unhandled comparison relation.");
  }
}

void CompareInstruction::InvertRelation() {
  switch (Relation) {
  case EQ:
    Relation = NE;
    break;
  case NE:
    Relation = EQ;
    break;
  case LT:
    Relation = GE;
    break;
  case GT:
    Relation = LE;
    break;
  case LE:
    Relation = GT;
    break;
  case GE:
    Relation = LT;
    break;
  default:
    break;
  }
}

void CompareInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "." << GetRelString() << "\t";
  std::cout << ValueString() << ", ";
  std::cout << LHS->ValueString() << ", ";
  std::cout << RHS->ValueString() << std::endl;
}

void CallInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  if (!ValueType.IsVoid())
    std::cout << ValueString() << ", ";
  std::cout << Name << "(";

  int i = 0;
  for (auto Arg : Arguments) {
    if (i > 0)
      std::cout << ", ";
    std::cout << Arg->ValueString();
    i++;
  }
  std::cout << ")" << std::endl;
}

std::string &JumpInstruction::GetTargetLabelName() { return Target->GetName(); }

void JumpInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  std::cout << "<" << Target->GetName() << ">" << std::endl;
}

std::string &BranchInstruction::GetTrueLabelName() {
  return TrueTarget->GetName();
}
std::string &BranchInstruction::GetFalseLabelName() {
  return FalseTarget->GetName();
}

void BranchInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  std::cout << Condition->ValueString() << ", ";
  std::cout << "<" << TrueTarget->GetName() << ">";
  if (FalseTarget)
    std::cout << ", <" << FalseTarget->GetName() << ">";
  std::cout << std::endl;
}

void ReturnInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  if (RetVal)
    std::cout << RetVal->ValueString() << std::endl;
}

void StackAllocationInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  std::cout << ValueString() << std::endl;
}

void GetElementPointerInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  std::cout << ValueString() << ", ";
  std::cout << Source->ValueString();
  std::string str = ", ";
  str += Index->ValueString();
  std::cout << str << std::endl;
}

void StoreInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  std::cout << "[" << Destination->ValueString() << "], ";
  std::cout << Source->ValueString() << std::endl;
}

void LoadInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  std::cout << ValueString() << ", ";
  std::cout << "[" << Source->ValueString();
  if (Offset)
    std::cout << " + " << Offset->ValueString();
  std::cout << "]" << std::endl;
}

void MemoryCopyInstruction::Print() const {
  std::cout << "\t" << AsString(InstKind) << "\t";
  std::cout << Dest->ValueString() << ", ";
  std::cout << Src->ValueString() << ", ";
  std::cout << N << std::endl;
}
