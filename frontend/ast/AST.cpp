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
  case Type::Int:
    return IRType(IRType::SINT);
  case Type::Double:
    return IRType(IRType::FP, 64);
  case Type::Composite:
    return IRType(IRType::STRUCT);
  default:
    assert(!"Invalid type");
    return IRType();
  }
}

static IRType GetIRTypeFromASTType(Type &CT) {
  IRType Result = GetIRTypeFromVK(CT.GetTypeVariant());

  if (Result.IsStruct()) {
    auto StructName = CT.GetName();
    Result.SetStructName(StructName);

    // convert each members AST type to IRType (recursive)
    for (auto &MemberASTType : CT.GetTypeList())
      Result.GetMemberTypes().push_back(GetIRTypeFromASTType(MemberASTType));
  }

  Result.SetPointerLevel(CT.GetPointerLevel());
  return Result;
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

Value *SwitchStatement::IRCodegen(IRFactory *IRF) {
  //   # generate code for Condition
  //   cmp.eq $cmp_res1, %Condition, case1_const
  //   br $cmp_res1, <case1_body>
  //   cmp.eq $cmp_res2, %Condition, case2_const
  //   br $cmp_res2, <case2_body>
  //   ...
  //   cmp.eq $cmp_resN, %Condition, caseN_const
  //   br $cmp_resN, <caseN_body>
  //   j <default_case>
  //
  // <case1_body>
  //   # generate case1 body
  //   # create "j <switch_end>" when break is used
  // ...
  // <caseN_body>
  //   # generate caseN body
  //   # create "j <switch_end>" when break is used
  // <default_case>
  //   # generate default case body
  // <switch_end>

  const auto FuncPtr = IRF->GetCurrentFunction();
  auto SwitchEnd = std::make_unique<BasicBlock>("switch_end", FuncPtr);
  auto DefaultCase = std::make_unique<BasicBlock>("switch_default", FuncPtr);

  auto Cond = Condition->IRCodegen(IRF);

  std::vector<std::unique_ptr<BasicBlock>> CaseBodies;

  for (auto &[Const, Statements] : Cases)
    if (!Statements.empty())
      CaseBodies.push_back(std::make_unique<BasicBlock>("switch_case", FuncPtr));

  // because of the fallthrough mechanism multiple cases could use the same
  // code block, CaseIdx keep track the current target basic block so falling
  // through cases could refer to it
  size_t CaseIdx = 0;
  for (auto &[Const, Statements] : Cases) {
    auto CMP_res = IRF->CreateCMP(CompareInstruction::EQ, Cond,
                                  IRF->GetConstant((uint64_t)Const));
    IRF->CreateBR(CMP_res, CaseBodies[CaseIdx].get());

    if (!Statements.empty())
      CaseIdx++;
  }

  IRF->CreateJUMP(DefaultCase.get());

  // Generating the bodies for the cases
  for (auto &[Const, Statements] : Cases) {
    if (!Statements.empty()) {
      IRF->InsertBB(std::move(CaseBodies.front()));
      for (auto &Statement : Statements) {
        Statement->IRCodegen(IRF);
        // If the statement is a "break" then insert jump to the default case
        // here, since cannot generate that jump simply, it would require context
        if (auto Break = dynamic_cast<BreakStatement*>(Statement.get()); Break != nullptr)
          IRF->CreateJUMP(SwitchEnd.get());
      }
      CaseBodies.erase(CaseBodies.begin());
    }
  }

  // Generate default case
  IRF->InsertBB(std::move(DefaultCase));
  for (auto &Statement : DefaultBody)
    Statement->IRCodegen(IRF);

  IRF->InsertBB(std::move(SwitchEnd));

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

  switch (T.GetReturnType()) {
  case Type::Composite:
    if (T.IsStruct()) {
      RetType = GetIRTypeFromASTType(T);
    } else
      assert(!"Other cases unhandled");
    break;
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
  auto ParamType = GetIRTypeFromASTType(Ty);
  auto Param = std::make_unique<FunctionParameter>(Name, ParamType);

  auto SA = IRF->CreateSA(Name, ParamType);
  IRF->AddToSymbolTable(Name, SA);
  IRF->CreateSTR(Param.get(), SA);
  IRF->Insert(std::move(Param));

  return nullptr;
}

Value *VariableDeclaration::IRCodegen(IRFactory *IRF) {
  auto Type = GetIRTypeFromASTType(AType);

  // If an array type, then change Type to reflect this
  if (AType.IsArray())
    Type.SetDimensions(AType.GetDimensions());

  // If we are in global scope, then its a global variable declaration
  std::vector<uint64_t> InitList;
  if (IRF->IsGlobalScope()) {
    // if the initialization is done by an initializer
    // FIXME: assuming 1 dimensional init list like "{ 1, 2 }", add support
    // for more complex case like "{ { 1, 2 }, { 3, 4 } }"
    if (auto InitListExpr = dynamic_cast<InitializerListExpression*>(Init.get());
        InitListExpr != nullptr) {
      for (auto &Expr : InitListExpr->GetExprList())
        if (auto ConstExpr = dynamic_cast<IntegerLiteralExpression*>(Expr.get());
            ConstExpr != nullptr)
          InitList.push_back(ConstExpr->GetUIntValue());
        else
          assert(!"Other types unhandled yet");
    }
    // initialized by const expression
    // FIXME: for now only IntegerLiteralExpression, add support for const
    // expressions like 1 + 2 - 4 * 12
    else {
      if (auto ConstExpr = dynamic_cast<IntegerLiteralExpression*>(Init.get());
          ConstExpr != nullptr) {
        InitList.push_back(ConstExpr->GetUIntValue());
      }
    }
    return IRF->CreateGlobalVar(Name, Type, std::move(InitList));
  }

  // Otherwise we are in a local scope of a function. Allocate space on
  // stack and update the local symbol table.
  auto SA = IRF->CreateSA(Name, Type);

  // TODO: revisit this
  if (Init)
    IRF->CreateSTR(Init->IRCodegen(IRF), SA);

  IRF->AddToSymbolTable(Name, SA);
  return SA;
}

Value *MemberDeclaration::IRCodegen(IRFactory *IRF) {
  return nullptr;
}

Value *StructDeclaration::IRCodegen(IRFactory *IRF) {
  return nullptr;
}

Value *EnumDeclaration::IRCodegen(IRFactory *IRF) {
  return nullptr;
}

Value *CallExpression::IRCodegen(IRFactory *IRF) {
  std::vector<Value *> Args;

  for (auto &Arg : Arguments) {
    auto ArgIR = Arg->IRCodegen(IRF);
    // if the generated IR result is a struct pointer, but the actual function
    // expects a struct by value, then issue an extra load
    if (ArgIR->GetTypeRef().IsStruct() && ArgIR->GetTypeRef().IsPTR() &&
        Arg->GetResultType().IsStruct() && !Arg->GetResultType().IsPointerType())
      ArgIR = IRF->CreateLD(ArgIR->GetType(), ArgIR);
    Args.push_back(ArgIR);
  }

  auto RetType = GetResultType().GetReturnType();

  IRType IRRetType;
  StackAllocationInstruction* StructTemp = nullptr;

  switch (RetType) {
  case Type::Int:
    IRRetType = IRType(IRType::SINT);
    break;
  case Type::Double:
    IRRetType = IRType(IRType::FP, 64);
    break;
  case Type::Void:
    IRRetType = IRType(IRType::NONE, 0);
    break;
  case Type::Composite: {
    IRRetType = GetIRTypeFromASTType(GetResultType());

    // If the return type is a struct, then also make a stack allocation
    // to use that as a temporary, where the result would be copied to after
    // the call
    StructTemp = IRF->CreateSA(Name + ".temp", IRRetType);
    break;
  }
  default:
    break;
  }

  // in case if the ret type was a struct, so StructTemp not nullptr
  if (StructTemp) {
    // make the call
    auto CallRes = IRF->CreateCALL(Name, Args, IRRetType);
    // issue a store using the freshly allocated temporary StructTemp
    IRF->CreateSTR(CallRes, StructTemp);
    return StructTemp;
  }

  return IRF->CreateCALL(Name, Args, IRRetType);
}

Value *ReferenceExpression::IRCodegen(IRFactory *IRF) {
  auto Local = IRF->GetSymbolValue(Identifier);

  if (Local && this->GetResultType().IsStruct())
    return Local;

  if (Local) {
    if (GetLValueness())
      return Local;
    else
      return IRF->CreateLD(Local->GetType(), Local);
  }

  auto GV = IRF->GetGlobalVar(Identifier);
  assert(GV && "Cannot be null");

  // If LValue, then return as a ptr to the global val
  if (GetLValueness())
    return GV;

  if (this->GetResultType().IsStruct())
    return GV;

  return IRF->CreateLD(GV->GetType(), GV);
}

Value *ArrayExpression::IRCodegen(IRFactory *IRF) {
  assert(BaseExpression && "BaseExpression cannot be NULL");
  auto BaseValue = BaseExpression->IRCodegen(IRF);
  assert(IndexExpression && "IndexExpression cannot be NULL");
  auto IndexValue = IndexExpression->IRCodegen(IRF);

  auto ArrayBaseType = BaseValue->GetType().GetBaseType();
  ArrayBaseType.IncrementPointerLevel();

  auto GEP = IRF->CreateGEP(ArrayBaseType, BaseValue, IndexValue);

  if (!GetLValueness())
    return IRF->CreateLD(ArrayBaseType, GEP);

  return GEP;
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

Value *StructMemberReference::IRCodegen(IRFactory *IRF) {
  assert(StructTypedExpression && "cannot be NULL");
  auto BaseValue = StructTypedExpression->IRCodegen(IRF);
  assert(BaseValue && "cannot be NULL");

  auto ExprType = BaseValue->GetType();
  assert(ExprType.IsStruct());

  auto IndexValue = IRF->GetConstant((uint64_t)MemberIndex);

  assert(ExprType.GetMemberTypes().size() > MemberIndex);

  // The result type is a pointer to the member type. Ex: referred member is
  // an i32 than an i32*.
  auto ResultType = ExprType.GetMemberTypes()[MemberIndex];
  ResultType.IncrementPointerLevel();
  auto GEP = IRF->CreateGEP(ResultType, BaseValue, IndexValue);

  if (GetLValueness())
    return GEP;

  auto ResultIRType = GetIRTypeFromASTType(this->GetResultType());

  return IRF->CreateLD(ResultIRType, GEP);
}

Value *UnaryExpression::IRCodegen(IRFactory *IRF) {
  auto E = Expr->IRCodegen(IRF);

  switch (GetOperationKind()) {
  case DEREF: {
    auto ResultType = E->GetType();
    return IRF->CreateLD(ResultType, E);
  }
  case POST_DECREMENT:
  case POST_INCREMENT: {
    // make the assumption that the expression E is an LValue which means
    // its basically a pointer, so it requires a load first for addition to work
    auto LoadedValType = E->GetTypeRef();
    LoadedValType.DecrementPointerLevel();
    auto LoadedExpr = IRF->CreateLD(LoadedValType, E);

    Instruction* AddSub;
    if (GetOperationKind() == POST_INCREMENT)
      AddSub = IRF->CreateADD(LoadedExpr, IRF->GetConstant((uint64_t)1));
    else
      AddSub = IRF->CreateSUB(LoadedExpr, IRF->GetConstant((uint64_t)1));

    IRF->CreateSTR(AddSub, E);
    return LoadedExpr;
  }
  default:
    assert(!"Unimplemented");
  }

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

    if (R->GetTypeRef().IsStruct())
      IRF->CreateMEMCOPY(L, R, R->GetTypeRef().GetByteSize());
    else
      IRF->CreateSTR(R, L);
    return R;
  }

  if (GetOperationKind() == ADD_ASSIGN || GetOperationKind() == SUB_ASSIGN ||
      GetOperationKind() == MUL_ASSIGN || GetOperationKind() == DIV_ASSIGN) {
    // Assignment right associative so generate R first
    auto R = Right->IRCodegen(IRF);
    auto L = Left->IRCodegen(IRF);

    if (!L || !R)
      return nullptr;

    if (R->GetTypeRef().IsStruct())
      IRF->CreateMEMCOPY(L, R, R->GetTypeRef().GetByteSize());
    else {
      Instruction *OperationResult = nullptr;

      switch (GetOperationKind()) {
      case ADD_ASSIGN:
        OperationResult = IRF->CreateADD(L, R);
        break;
      case SUB_ASSIGN:
        OperationResult = IRF->CreateSUB(L, R);
        break;
      case MUL_ASSIGN:
        OperationResult = IRF->CreateMUL(L, R);
        break;
      case DIV_ASSIGN:
        OperationResult = IRF->CreateDIV(L, R);
        break;
      default:
        assert(!"Unreachable");
      }
      // TODO: Revisit this. Its not necessary guaranteed that it will be a load
      // for now it seems fine
      auto Load = dynamic_cast<LoadInstruction*>(L);
      assert(Load);
      IRF->CreateSTR(OperationResult, Load->GetMemoryLocation());
      return OperationResult;
    }
  }

  auto L = Left->IRCodegen(IRF);
  auto R = Right->IRCodegen(IRF);

  if (!L || !R)
    return nullptr;

  // if the left operand is a constant
  if (L->IsConstant() && !R->IsConstant()) {
    // and if its a commutative operation
    switch (GetOperationKind()) {
    case ADD:
    case MUL:
    case AND:
    case EQ:
    case NE:
      // then swap the operands, since most architecture supports immediate
      // as the last operand. Ex.: AArch64 add x0, x1, #123 not add x0, #123, x1
      std::swap(L, R);
      break;
    default:
      break;
    }
  }

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
