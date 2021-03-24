#include "AST.hpp"
#include "Type.hpp"
#include <memory>

//=--------------------------------------------------------------------------=//
//=------------------------- IR Codegen functions ---------------------------=//
//=--------------------------------------------------------------------------=//

static IRType GetIRTypeFromVK(Type::VariantKind VK) {
  switch (VK) {
  case Type::Char:
    return IRType(IRType::SINT, 8);
    break;
  case Type::Int:
    return IRType(IRType::SINT);
    break;
  case Type::Double:
    return IRType(IRType::FP, 64);
    break;
  default:
    assert(!"Invalid type");
    return IRType();
  }
}

Value *IfStatement::IRCodegen(IRFactory *IRF) {
  // if there is no else clause, then IR should be something like:
  //    # generate code for Condition
  //    # if the Condition is a CMP instruction, then revert its
  //    # relation otherwise insert another CMP like this:
  //    # cmp.eq $c, $Condition, 0 # true if Condition false
  //    br $c, <if_end>
  // <if_true>
  //    # generate code for IfBody
  // <if_end>
  //
  // If there is also an else branch then:
  //    # generate code for Condition
  //    # if the Condition is a CMP instruction, then revert its
  //    # relation otherwise insert another CMP like this:
  //    # cmp.eq $c, $Condition, 0 # true if Condition false
  //    br $c, <else>
  // <if_true>
  //    # generate code for IfBody
  //    j <if_end>
  // <else>
  //    # generate code for ElseBody
  //    j <if_end>
  // <if_end>

  const bool HaveElse = ElseBody != nullptr;
  const auto FuncPtr = IRF->GetCurrentFunction();

  std::unique_ptr<BasicBlock> Else;
  if (HaveElse)
    Else = std::make_unique<BasicBlock>("if_else", FuncPtr);

  auto IfEnd = std::make_unique<BasicBlock>("if_end", FuncPtr);

  auto Cond = Condition->IRCodegen(IRF);

  // if Condition was a compare instruction then just revert its relation
  if (auto CMP = dynamic_cast<CompareInstruction *>(Cond); CMP != nullptr) {
    CMP->InvertRelation();
    IRF->CreateBR(Cond, HaveElse ? Else.get() : IfEnd.get());
  } else {
    auto Cmp = IRF->CreateCMP(CompareInstruction::EQ, Cond,
                              IRF->GetConstant((uint64_t)0));
    IRF->CreateBR(Cmp, HaveElse ? Else.get() : IfEnd.get());
  }

  // if true
  auto IfTrue = std::make_unique<BasicBlock>("if_true", FuncPtr);
  IRF->InsertBB(std::move(IfTrue));
  IfBody->IRCodegen(IRF);
  IRF->CreateJUMP(IfEnd.get());

  if (HaveElse) {
    IRF->InsertBB(std::move(Else));
    ElseBody->IRCodegen(IRF);
    IRF->CreateJUMP(IfEnd.get());
  }

  IRF->InsertBB(std::move(IfEnd));
  return nullptr;
}

Value *WhileStatement::IRCodegen(IRFactory *IRF) {
  //  <loop_header>
  //    # generate code for the Condition
  //    # if the Condition is a CMP instruction, then revert its
  //    # relation otherwise insert another CMP like this:
  //    # cmp.eq $c, $Condition, 0    # true if Condition false
  //    br $condition, <loop_end>     # goto loop_end if condition false
  //  <loop_body>
  //    # generate conde for the Body
  //    j <loop_header>
  //  <loop_end>

  const auto FuncPtr = IRF->GetCurrentFunction();
  auto Header = std::make_unique<BasicBlock>("loop_header", FuncPtr);
  auto LoopBody = std::make_unique<BasicBlock>("loop_body", FuncPtr);
  auto LoopEnd = std::make_unique<BasicBlock>("loop_end", FuncPtr);
  auto HeaderPtr = Header.get();

  IRF->CreateJUMP(Header.get());

  IRF->InsertBB(std::move(Header));
  auto Cond = Condition->IRCodegen(IRF);

  // if Condition was a compare instruction then just revert its relation
  if (auto CMP = dynamic_cast<CompareInstruction *>(Cond); CMP != nullptr) {
    CMP->InvertRelation();
    IRF->CreateBR(Cond, LoopEnd.get());
  } else {
    auto Cmp = IRF->CreateCMP(CompareInstruction::EQ, Cond,
                              IRF->GetConstant((uint64_t)0));
    IRF->CreateBR(Cmp, LoopEnd.get());
  }

  IRF->InsertBB(std::move(LoopBody));
  Body->IRCodegen(IRF);
  IRF->CreateJUMP(HeaderPtr);

  IRF->InsertBB(std::move(LoopEnd));

  return nullptr;
}

Value *ForStatement::IRCodegen(IRFactory *IRF) {
  // Similar solution to WhileStatement. The difference is that here the
  // initialization part has to be generated before the loop_header basicblock
  // and also inserting the increment expression before the backward jump to the
  // loop_header

  const auto FuncPtr = IRF->GetCurrentFunction();
  auto Header = std::make_unique<BasicBlock>("loop_header", FuncPtr);
  auto LoopBody = std::make_unique<BasicBlock>("loop_body", FuncPtr);
  auto LoopEnd = std::make_unique<BasicBlock>("loop_end", FuncPtr);
  auto HeaderPtr = Header.get();

  // Generating code for the initializing expression and adding and explicit
  // unconditional jump to the loop header basic block
  Init->IRCodegen(IRF);
  IRF->CreateJUMP(Header.get());

  // Inserting the loop header basicblock and generating the code for the
  // loop condition
  IRF->InsertBB(std::move(Header));
  auto Cond = Condition->IRCodegen(IRF);

  // if Condition was a compare instruction then just revert its relation
  if (auto CMP = dynamic_cast<CompareInstruction *>(Cond); CMP != nullptr) {
    CMP->InvertRelation();
    IRF->CreateBR(Cond, LoopEnd.get());
  } else {
    auto CMPEQ = IRF->CreateCMP(CompareInstruction::EQ, Cond,
                              IRF->GetConstant((uint64_t)0));
    IRF->CreateBR(CMPEQ, LoopEnd.get());
  }

  IRF->InsertBB(std::move(LoopBody));
  Body->IRCodegen(IRF);
  Increment->IRCodegen(IRF); // generating loop increment code here
  IRF->CreateJUMP(HeaderPtr);

  IRF->InsertBB(std::move(LoopEnd));

  return nullptr;
}

Value *CompoundStatement::IRCodegen(IRFactory *IRF) {
  for (auto &Declaration : Declarations)
    Declaration->IRCodegen(IRF);
  for (auto &Statement : Statements)
    Statement->IRCodegen(IRF);
  return nullptr;
}

Value *ExpressionStatement::IRCodegen(IRFactory *IRF) {
  return Expr->IRCodegen(IRF);
}

Value *ReturnStatement::IRCodegen(IRFactory *IRF) {
  if (!ReturnValue.has_value())
    return IRF->CreateRET(nullptr);

  auto RetVal = ReturnValue.value()->IRCodegen(IRF);

  if (RetVal == nullptr)
    return nullptr;

  return IRF->CreateRET(RetVal);
}

Value *FunctionDeclaration::IRCodegen(IRFactory *IRF) {
  IRF->SetGlobalScope(false);

  IRType RetType;

  switch (Type.GetReturnType()) {
  case Type::Char:
    RetType = IRType(IRType::SINT, 8);
    break;
  case Type::Int:
    RetType = IRType(IRType::SINT);
    break;
  case Type::Double:
    RetType = IRType(IRType::FP, 64);
    break;
  case Type::Void:
    RetType = IRType(IRType::NONE);
    break;
  default:
    assert(!"Invalid function return type.");
    break;
  }

  IRF->CreateNewFunction(Name, RetType);

  for (auto &Argument : Arguments)
    Argument->IRCodegen(IRF);

  Body->IRCodegen(IRF);
  return nullptr;
}

Value *FunctionParameterDeclaration::IRCodegen(IRFactory *IRF) {
  auto SA = IRF->CreateSA(Name, GetIRTypeFromVK(Ty.GetTypeVariant()));
  IRF->AddToSymbolTable(Name, SA);

  IRType ParamType = GetIRTypeFromVK(Ty.GetTypeVariant());
  auto Param =
      std::make_unique<FunctionParameter>(FunctionParameter(Name, ParamType));

  IRF->CreateSTR(Param.get(), SA);
  IRF->Insert(std::move(Param));
  return nullptr;
}

Value *VariableDeclaration::IRCodegen(IRFactory *IRF) {
  auto Type = GetIRTypeFromVK(AType.GetTypeVariant());

  // If an array type, then change Type to reflect this
  if (AType.IsArray()) {
    unsigned ElementNumber = 1;

    for (auto Dim : AType.GetDimensions())
      ElementNumber *= Dim;

    Type.SetNumberOfElements(ElementNumber);
  }

  // If we are in global scope, then its a global variable declaration
  if (IRF->IsGlobalScope())
    return IRF->CreateGlobalVar(Name, Type);

  // Otherwise we are in a localscope of a function. Allocate space on
  // stack and update the local symbol table.
  auto SA = IRF->CreateSA(Name, Type);
  IRF->AddToSymbolTable(Name, SA);
  return SA;
}

Value *CallExpression::IRCodegen(IRFactory *IRF) {
  std::vector<Value *> Args;

  for (auto &Arg : Arguments)
    Args.push_back(Arg->IRCodegen(IRF));

  auto RetType = GetResultType().GetFunctionType().GetReturnType();

  IRType IRretType;

  switch (RetType) {
  case Type::Int:
    IRretType = IRType(IRType::SINT);
    break;
  case Type::Double:
    IRretType = IRType(IRType::FP, 64);
    break;
  case Type::Void:
    IRretType = IRType(IRType::NONE, 0);
    break;
  default:
    break;
  }

  return IRF->CreateCALL(Name, Args, IRretType);
}

Value *ReferenceExpression::IRCodegen(IRFactory *IRF) {
  auto Local = IRF->GetSymbolValue(Identifier);

  if (Local) {
    if (GetLValueness())
      return Local;
    else
      return IRF->CreateLD(Local->GetType(), Local);
  }

  auto GV = IRF->GetGlobalVar(Identifier);

  // If LValue, then return as a ptr to the global val
  if (GetLValueness())
    return GV;

  return IRF->CreateLD(GV->GetType(), GV);
}

Value *ArrayExpression::IRCodegen(IRFactory *IRF) {
  // If used as RValue
  //    # calc index expression
  //    mul $final_index, $index, sizeof($array.basetype)
  //    ld  $rv, [$arr + #final_index]
  //
  // If used as LValue
  // # generate Index
  //    mul $offset, $Index, sizeof(Array[0])
  //    str [$array_base + $offset], $R

  // FIXME: for now just assume only 1 dimensional arrays
  auto Index = IndexExpressions[0]->IRCodegen(IRF);

  auto ArrayBaseType = Index->GetType().GetBaseType();

  auto SizeOfArrayBaseType = IRF->GetConstant(ArrayBaseType.GetByteSize());
  auto FinalIndex = IRF->CreateMUL(Index, SizeOfArrayBaseType);

  // Return a ptr to the LValue
  if (GetLValueness()) {
    auto ID = Identifier.GetString();
    auto LocalVal = IRF->GetSymbolValue(ID);
    auto GlobalVal = IRF->GetGlobalVar(ID);

    auto PtrToElement =
        IRF->CreateADD(LocalVal ? LocalVal : GlobalVal, FinalIndex);

    PtrToElement->GetType().SetToPointerKind();

    return PtrToElement;
  }

  // return the RValue
  auto Element = IRF->CreateLD(
      ArrayBaseType, IRF->GetSymbolValue(Identifier.GetString()), FinalIndex);

  return Element;
}

Value *ImplicitCastExpression::IRCodegen(IRFactory *IRF) {
  auto SourceTypeVariant = CastableExpression->GetResultType().GetTypeVariant();
  auto DestTypeVariant = GetResultType().GetTypeVariant();
  auto Val = CastableExpression->IRCodegen(IRF);

  if (SourceTypeVariant == Type::Int && DestTypeVariant == Type::Double)
    return IRF->CreateITOF(Val, 32);
  else if (SourceTypeVariant == Type::Double && DestTypeVariant == Type::Int)
    return IRF->CreateFTOI(Val, 64);
  else if (SourceTypeVariant == Type::Char && DestTypeVariant == Type::Int)
    return IRF->CreateSEXT(Val, 32);
  else if (SourceTypeVariant == Type::Int && DestTypeVariant == Type::Char)
    return IRF->CreateTRUNC(Val, 8);
  else
    assert(!"Invaid conversion.");

  return nullptr;
}

Value *BinaryExpression::IRCodegen(IRFactory *IRF) {
  if (GetOperationKind() == ANDL) {
    // goal IR:
    //    # L generated here
    //    sa $result
    //    cmp.eq $c1, $L, 0
    //    br $c1, <false>
    // <test_R>
    //    # R generated here
    //    cmp.eq $c2, $R, 0
    //    br $c2, <false>
    // <true>
    //    str [$result], 1
    //    j <end>
    // <false>
    //    str [$result], 0
    // <end>

    const auto FuncPtr = IRF->GetCurrentFunction();

    auto TestRhsBB = std::make_unique<BasicBlock>("test_RHS", FuncPtr);
    auto TrueBB = std::make_unique<BasicBlock>("true", FuncPtr);
    auto FalseBB = std::make_unique<BasicBlock>("false", FuncPtr);
    auto FinalBB = std::make_unique<BasicBlock>("final", FuncPtr);

    auto L = Left->IRCodegen(IRF);

    // LHS Test
    auto Result = IRF->CreateSA("result", IRType::CreateBool());

    // if L was a compare instruction then just revert its relation
    if (auto LCMP = dynamic_cast<CompareInstruction *>(L); LCMP != nullptr) {
      LCMP->InvertRelation();
      IRF->CreateBR(L, FalseBB.get());
    } else {
      auto LHSTest = IRF->CreateCMP(CompareInstruction::EQ, L,
                                    IRF->GetConstant((uint64_t)0));
      IRF->CreateBR(LHSTest, FalseBB.get());
    }

    // RHS Test
    IRF->InsertBB(std::move(TestRhsBB));
    auto R = Right->IRCodegen(IRF);

    // if R was a compare instruction then just revert its relation
    if (auto RCMP = dynamic_cast<CompareInstruction *>(R); RCMP != nullptr) {
      RCMP->InvertRelation();
      IRF->CreateBR(R, FalseBB.get());
    } else {
      auto RHSTest = IRF->CreateCMP(CompareInstruction::EQ, R,
                                    IRF->GetConstant((uint64_t)0));
      IRF->CreateBR(RHSTest, FalseBB.get());
    }

    // TRUE
    IRF->InsertBB(std::move(TrueBB));
    IRF->CreateSTR(IRF->GetConstant((uint64_t)1), Result);
    IRF->CreateJUMP(FinalBB.get());

    // FALSE
    IRF->InsertBB(std::move(FalseBB));
    IRF->CreateSTR(IRF->GetConstant((uint64_t)0), Result);
    IRF->CreateJUMP(FinalBB.get());

    IRF->InsertBB(std::move(FinalBB));

    return Result;
  }

  if (GetOperationKind() == ASSIGN) {
    // Assignment right associative so generate R first
    auto R = Right->IRCodegen(IRF);
    auto L = Left->IRCodegen(IRF);

    if (!L || !R)
      return nullptr;

    IRF->CreateSTR(R, L);
    return R;
  }

  auto L = Left->IRCodegen(IRF);
  auto R = Right->IRCodegen(IRF);

  if (!L || !R)
    return nullptr;

  switch (GetOperationKind()) {
  case ADD:
    return IRF->CreateADD(L, R);
  case SUB:
    return IRF->CreateSUB(L, R);
  case MUL:
    return IRF->CreateMUL(L, R);
  case DIV:
    return IRF->CreateDIV(L, R);
  case MOD:
    return IRF->CreateMOD(L, R);
  case AND:
    return IRF->CreateAND(L, R);
  case EQ:
    return IRF->CreateCMP(CompareInstruction::EQ, L, R);
  case LT:
    return IRF->CreateCMP(CompareInstruction::LT, L, R);
  case GT:
    return IRF->CreateCMP(CompareInstruction::GT, L, R);
  case NE:
    return IRF->CreateCMP(CompareInstruction::NE, L, R);
  default:
    assert(!"Unhandled binary instruction type");
    break;
  }
}

Value *IntegerLiteralExpression::IRCodegen(IRFactory *IRF) {
  return IRF->GetConstant(IntValue);
}

Value *FloatLiteralExpression::IRCodegen(IRFactory *IRF) {
  return IRF->GetConstant(FPValue);
}

Value *TranslationUnit::IRCodegen(IRFactory *IRF) {
  for (auto &Declaration : Declarations) {
    IRF->SetGlobalScope();
    if (auto Decl = Declaration->IRCodegen(IRF); Decl != nullptr)
      IRF->AddGlobalVariable(Decl);
  }
  return nullptr;
}
