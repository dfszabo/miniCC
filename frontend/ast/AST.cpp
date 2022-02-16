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
  case Type::UnsignedChar:
    return IRType(IRType::UINT, 8);
  case Type::Short:
    return IRType(IRType::SINT, 16);
  case Type::UnsignedShort:
    return IRType(IRType::UINT, 16);
  case Type::Int:
    return IRType(IRType::SINT);
  case Type::UnsignedInt:
    return IRType(IRType::UINT);
  case Type::Long:
    return IRType(IRType::SINT, 64);
  case Type::UnsignedLong:
    return IRType(IRType::UINT, 64);
  case Type::LongLong:
    return IRType(IRType::SINT, 64);
  case Type::UnsignedLongLong:
    return IRType(IRType::UINT, 64);
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
  if (CT.IsArray()) {
    Result.SetDimensions(CT.GetDimensions());
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
      CaseBodies.push_back(
          std::make_unique<BasicBlock>("switch_case", FuncPtr));

  IRF->GetBreaksEndBBsTable().push_back(SwitchEnd.get());

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
      }
      CaseBodies.erase(CaseBodies.begin());
    }
  }

  // Generate default case
  IRF->InsertBB(std::move(DefaultCase));
  for (auto &Statement : DefaultBody)
    Statement->IRCodegen(IRF);

  IRF->GetBreaksEndBBsTable().erase(IRF->GetBreaksEndBBsTable().end() - 1);
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

  IRF->GetBreaksEndBBsTable().push_back(LoopEnd.get());
  IRF->InsertBB(std::move(LoopBody));
  Body->IRCodegen(IRF);
  IRF->GetBreaksEndBBsTable().erase(IRF->GetBreaksEndBBsTable().end() - 1);
  IRF->CreateJUMP(HeaderPtr);

  IRF->InsertBB(std::move(LoopEnd));

  return nullptr;
}

Value *ForStatement::IRCodegen(IRFactory *IRF) {
  // Similar solution to WhileStatement. The difference is that here the
  // initialization part has to be generated before the loop_header basicblock
  // and also inserting the increment expression before the backward jump to the
  // loop_header
  // TODO: Add support for break statement
  const auto FuncPtr = IRF->GetCurrentFunction();
  auto Header = std::make_unique<BasicBlock>("loop_header", FuncPtr);
  auto LoopBody = std::make_unique<BasicBlock>("loop_body", FuncPtr);
  auto LoopIncrement = std::make_unique<BasicBlock>("loop_increment", FuncPtr);
  auto LoopEnd = std::make_unique<BasicBlock>("loop_end", FuncPtr);
  auto HeaderPtr = Header.get();

  // Generating code for the initializing expression or the variable declaration
  // and adding and explicit unconditional jump to the loop header basic block
  if (Init)
    Init->IRCodegen(IRF);
  else
    VarDecl->IRCodegen(IRF);
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
  // Push entry
  IRF->GetLoopIncrementBBsTable().push_back(LoopIncrement.get());
  IRF->GetBreaksEndBBsTable().push_back(LoopEnd.get());
  Body->IRCodegen(IRF);
  // Pop entry
  IRF->GetBreaksEndBBsTable().erase(IRF->GetBreaksEndBBsTable().end() - 1);
  IRF->GetLoopIncrementBBsTable().erase(IRF->GetLoopIncrementBBsTable().end() -
                                        1);
  IRF->CreateJUMP(LoopIncrement.get());
  IRF->InsertBB(std::move(LoopIncrement));
  Increment->IRCodegen(IRF); // generating loop increment code here
  IRF->CreateJUMP(HeaderPtr);

  IRF->InsertBB(std::move(LoopEnd));

  return nullptr;
}

Value *CompoundStatement::IRCodegen(IRFactory *IRF) {
  for (auto &Statement : Statements)
    Statement->IRCodegen(IRF);
  return nullptr;
}

Value *ExpressionStatement::IRCodegen(IRFactory *IRF) {
  return Expr->IRCodegen(IRF);
}

Value *ReturnStatement::IRCodegen(IRFactory *IRF) {
  auto RetNum = IRF->GetCurrentFunction()->GetReturnsNumber();
  IRF->GetCurrentFunction()->SetReturnsNumber(RetNum - 1);

  bool HasRetVal =
      ReturnValue.has_value() && !IRF->GetCurrentFunction()->IsRetTypeVoid();

  Value *RetVal = HasRetVal ? ReturnValue.value()->IRCodegen(IRF) : nullptr;

  if (IRF->GetCurrentFunction()->HasMultipleReturn()) {
    if (HasRetVal)
      IRF->CreateSTR(RetVal, IRF->GetCurrentFunction()->GetReturnValue());
    return IRF->CreateJUMP(nullptr);
  }
  return IRF->CreateRET(RetVal);
}

Value *FunctionDeclaration::IRCodegen(IRFactory *IRF) {
  IRF->SetGlobalScope(false);

  IRType RetType;
  IRType ParamType;
  std::unique_ptr<FunctionParameter> ImplicitStructPtr = nullptr;
  bool NeedIgnore = false;

  switch (T.GetReturnType()) {
  case Type::Composite:
    if (T.IsStruct()) {
      RetType = GetIRTypeFromASTType(T);

      // in case the struct is to big to pass by value
      if (!RetType.IsPTR() &&
          (RetType.GetByteSize() * 8) > IRF->GetTargetMachine()
                                            ->GetABI()
                                            ->GetMaxStructSizePassedByValue()) {
        NeedIgnore = true;
        ParamType = RetType;
        ParamType.IncrementPointerLevel();
        // then the return type is void and the struct to be returned will be
        // allocated by the callers and a pointer is passed to the function
        // as an extra argument
        RetType = IRType(IRType::NONE);

        // on the same note create the extra struct pointer operand
        auto ParamName = "struct." + ParamType.GetStructName();
        ImplicitStructPtr =
            std::make_unique<FunctionParameter>(ParamName, ParamType, true);
      }
    } else
      assert(!"Other cases unhandled");
    break;
  case Type::Char:
    RetType = IRType(IRType::SINT, 8);
    break;
  case Type::UnsignedChar:
    RetType = IRType(IRType::UINT, 8);
    break;
  case Type::Short:
    RetType = IRType(IRType::SINT, 16);
    break;
  case Type::UnsignedShort:
    RetType = IRType(IRType::UINT, 16);
    break;
  case Type::Int:
    RetType = IRType(IRType::SINT);
    break;
  case Type::UnsignedInt:
    RetType = IRType(IRType::UINT);
    break;
  case Type::Long:
    RetType = IRType(IRType::SINT, 64);
    break;
  case Type::UnsignedLong:
    RetType = IRType(IRType::UINT, 64);
    break;
  case Type::LongLong:
    RetType = IRType(IRType::SINT, 64);
    break;
  case Type::UnsignedLongLong:
    RetType = IRType(IRType::UINT, 64);
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
  IRF->GetCurrentFunction()->SetReturnsNumber(ReturnsNumber);

  if (Body == nullptr) {
    IRF->GetCurrentFunction()->SetToDeclarationOnly();
    return nullptr;
  }

  if (ImplicitStructPtr) {
    IRF->AddToSymbolTable(ImplicitStructPtr->GetName(),
                          ImplicitStructPtr.get());
    IRF->Insert(std::move(ImplicitStructPtr));
  }

  for (auto &Argument : Arguments)
    Argument->IRCodegen(IRF);

  // iterate over the statements and find returns
  if (NeedIgnore) {
    auto CS = dynamic_cast<CompoundStatement *>(Body.get());
    assert(CS);
    for (auto &Stmt : CS->GetStatements())
      if (Stmt->IsRet()) {
        auto RetStmt = dynamic_cast<ReturnStatement *>(Stmt.get());
        auto RefExpr =
            dynamic_cast<ReferenceExpression *>(RetStmt->GetRetVal().get());
        if (RefExpr)
          IRF->GetCurrentFunction()->SetIgnorableStructVarName(
              RefExpr->GetIdentifier());
      }
  }

  // if there are multiple returns, then create a local variable on the stack
  // which will hold the different return values
  auto HasMultipleReturn = ReturnsNumber > 1 && !RetType.IsVoid();
  if (HasMultipleReturn)
    IRF->GetCurrentFunction()->SetReturnValue(
        IRF->CreateSA(Name + ".return", RetType));

  Body->IRCodegen(IRF);

  // patching JUMP -s with nullptr destination to make them point to the last BB
  if (HasMultipleReturn) {
    auto BBName = Name + "_end";
    auto RetBB =
        std::make_unique<BasicBlock>(BBName, IRF->GetCurrentFunction());
    auto RetBBPtr = RetBB.get();
    IRF->InsertBB(std::move(RetBB));
    auto RetVal = IRF->GetCurrentFunction()->GetReturnValue();
    auto LD = IRF->CreateLD(RetVal->GetType(), RetVal);
    IRF->CreateRET(LD);

    for (auto &BB : IRF->GetCurrentFunction()->GetBasicBlocks())
      for (auto &Instr : BB->GetInstructions())
        if (auto Jump = dynamic_cast<JumpInstruction *>(Instr.get());
            Jump && Jump->GetTargetBB() == nullptr)
          Jump->SetTargetBB(RetBBPtr);
  }

  // if its a void function without return statement, then add one
  if (ReturnsNumber == 0 && RetType.IsVoid())
    IRF->CreateRET(nullptr);

  return nullptr;
}

Value *ContinueStatement::IRCodegen(IRFactory *IRF) {
  return IRF->CreateJUMP(IRF->GetLoopIncrementBBsTable().back());
}

Value *BreakStatement::IRCodegen(IRFactory *IRF) {
  assert(IRF->GetBreaksEndBBsTable().size() > 0);
  return IRF->CreateJUMP(IRF->GetBreaksEndBBsTable().back());
}

Value *FunctionParameterDeclaration::IRCodegen(IRFactory *IRF) {
  auto ParamType = GetIRTypeFromASTType(Ty);

  // if the param is a struct and too big to passed by value then change it
  // to a struct pointer, because that is how it will be passed by callers
  if (ParamType.IsStruct() && !ParamType.IsPTR() &&
      (ParamType.GetByteSize() * 8) >
          IRF->GetTargetMachine()->GetABI()->GetMaxStructSizePassedByValue())
    ParamType.IncrementPointerLevel();

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
    // FIXME: assuming max 2 dimensional init list like "{ { 1, 2 }, { 3, 4 } }"
    // add support for arbitrary dimension
    if (auto InitListExpr =
            dynamic_cast<InitializerListExpression *>(Init.get());
        InitListExpr != nullptr) {
      for (auto &Expr : InitListExpr->GetExprList())
        if (auto ConstExpr =
                dynamic_cast<IntegerLiteralExpression *>(Expr.get());
            ConstExpr != nullptr) {
          InitList.push_back(ConstExpr->GetUIntValue());
        } else if (auto InitListExpr =
                       dynamic_cast<InitializerListExpression *>(Expr.get());
                   InitListExpr != nullptr) {
          for (auto &Expr : InitListExpr->GetExprList())
            if (auto ConstExpr =
                    dynamic_cast<IntegerLiteralExpression *>(Expr.get());
                ConstExpr != nullptr)
              InitList.push_back(ConstExpr->GetUIntValue());
            else
              assert(!"Other types unhandled yet");
        }
    }
    // initialized by const expression
    // FIXME: for now only IntegerLiteralExpression, add support for const
    // expressions like 1 + 2 - 4 * 12
    else {
      if (auto ConstExpr = dynamic_cast<IntegerLiteralExpression *>(Init.get());
          ConstExpr != nullptr) {
        InitList.push_back(ConstExpr->GetUIntValue());
      }
      // string literal case like: char *str = "Hello World"
      else if (auto StringLitExpr =
                   dynamic_cast<StringLiteralExpression *>(Init.get());
               StringLitExpr != nullptr) {
        // generate code for the string literal -> create a global var for it
        auto GVStr = StringLitExpr->IRCodegen(IRF);

        // increase to pointer level since now the pointer to the data is stored
        // and not the data itself
        Type.IncrementPointerLevel();
        return IRF->CreateGlobalVar(Name, Type, GVStr);
      }
    }
    return IRF->CreateGlobalVar(Name, Type, std::move(InitList));
  }

  if (IRF->GetCurrentFunction()->GetIgnorableStructVarName() == Name) {
    auto ParamValue =
        IRF->GetCurrentFunction()
            ->GetParameters()
                [IRF->GetCurrentFunction()->GetParameters().size() - 1]
            .get();
    IRF->AddToSymbolTable(Name, ParamValue);
    return ParamValue;
  }

  // Otherwise we are in a local scope of a function. Allocate space on
  // stack and update the local symbol table.
  auto SA = IRF->CreateSA(Name, Type);

  // TODO: revisit this
  if (Init) {
    // If initialized with initializer list then assuming its only 1 dimensional
    // and only contain integer literal expressions.
    if (auto InitListExpr =
            dynamic_cast<InitializerListExpression *>(Init.get());
        InitListExpr != nullptr) {
      unsigned LoopCounter = 0;
      for (auto &Expr : InitListExpr->GetExprList()) {
        if (auto ConstExpr =
                dynamic_cast<IntegerLiteralExpression *>(Expr.get());
            ConstExpr != nullptr) {
          // basically storing each entry to the right stack area
          // TODO: problematic for big arrays, Clang and GCC create a global
          // array to store there the initial values and use memcopy
          auto ResultType = SA->GetType();
          ResultType.ReduceDimension();

          if (ResultType.GetPointerLevel() == 0)
            ResultType.IncrementPointerLevel();

          auto GEP = IRF->CreateGEP(ResultType, SA,
                                    IRF->GetConstant((uint64_t)LoopCounter));
          IRF->CreateSTR(IRF->GetConstant((uint64_t)ConstExpr->GetUIntValue()),
                         GEP);
        }
        LoopCounter++;
      }
    } else {
      auto InitExpr = Init->IRCodegen(IRF);

      if (InitExpr->GetType().IsStruct())
        IRF->CreateMEMCOPY(SA, InitExpr, InitExpr->GetType().GetByteSize());
      else
        IRF->CreateSTR(InitExpr, SA);
    }
  }

  IRF->AddToSymbolTable(Name, SA);
  return SA;
}

Value *MemberDeclaration::IRCodegen(IRFactory *IRF) { return nullptr; }

Value *StructDeclaration::IRCodegen(IRFactory *IRF) { return nullptr; }

Value *EnumDeclaration::IRCodegen(IRFactory *IRF) { return nullptr; }

Value *CallExpression::IRCodegen(IRFactory *IRF) {
  std::vector<Value *> Args;

  for (auto &Arg : Arguments) {
    auto ArgIR = Arg->IRCodegen(IRF);
    // if the generated IR result is a struct pointer, but the actual function
    // expects a struct by value, then issue an extra load
    if (ArgIR->GetTypeRef().IsStruct() && ArgIR->GetTypeRef().IsPTR() &&
        Arg->GetResultType().IsStruct() &&
        !Arg->GetResultType().IsPointerType()) {
      // if it possible to pass it by value then issue a load first otherwise
      // it passed by pointer which already is
      if (!((ArgIR->GetTypeRef().GetByteSize() * 8) >
            IRF->GetTargetMachine()->GetABI()->GetMaxStructSizePassedByValue()))
        ArgIR = IRF->CreateLD(ArgIR->GetType(), ArgIR);
    }

    // if the pointers level does not match then issue loads until it will
    while (
        (Arg->GetResultType().IsPointerType() && ArgIR->GetTypeRef().IsPTR()) &&
        (Arg->GetResultType().GetPointerLevel() <
         ArgIR->GetTypeRef().GetPointerLevel())) {
      ArgIR = IRF->CreateLD(ArgIR->GetType(), ArgIR);
    }

    Args.push_back(ArgIR);
  }

  auto RetType = GetResultType().GetReturnType();

  IRType IRRetType;
  StackAllocationInstruction *StructTemp = nullptr;
  bool IsRetChanged = false;
  int ImplicitStructIndex = -1;

  switch (RetType) {
  case Type::Int:
    IRRetType = IRType(IRType::SINT);
    break;
  case Type::UnsignedInt:
    IRRetType = IRType(IRType::UINT);
    break;
  case Type::Long:
    IRRetType = IRType(IRType::SINT, 64);
    break;
  case Type::UnsignedLong:
    IRRetType = IRType(IRType::UINT, 64);
    break;
  case Type::LongLong:
    IRRetType = IRType(IRType::SINT, 64);
    break;
  case Type::UnsignedLongLong:
    IRRetType = IRType(IRType::UINT, 64);
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

    // check if the call expression is returning a non pointer struct which is
    // to big to be returned back. In this case the called function were already
    // changed to expect an extra struct pointer parameter and use that and
    // also its no longer returning anything, it returns type now void
    // In this case we need to
    //  -allocating space for the struct, which actually do not needed since
    //   at this point its already done above (StructTemp)
    //  -adding extra parameter which is a pointer to this allocated struct
    //  -change the returned value to this newly allocated struct pointer
    //  (even though nothing is returned, doing this so subsequent instructions
    //  can use this struct instead)
    //
    //  FIXME: maybe an extra load will required since its now a struct pointer
    // but originally the return is a struct (not a pointer)
    if (!(!IRRetType.IsPTR() && (IRRetType.GetByteSize() * 8) >
                                    IRF->GetTargetMachine()
                                        ->GetABI()
                                        ->GetMaxStructSizePassedByValue()))
      break; // actually checking the opposite and break if its true

    IsRetChanged = true;
    ImplicitStructIndex = Args.size();
    Args.push_back(StructTemp);
    IRRetType = IRType::NONE;
    break;
  }
  default:
    assert(!"Unreachable");
    break;
  }

  assert(!IRRetType.IsInvalid() && "Must be a valid type");

  // in case if the ret type was a struct, so StructTemp not nullptr
  if (StructTemp) {
    // make the call
    auto CallRes = IRF->CreateCALL(Name, Args, IRRetType, ImplicitStructIndex);
    // issue a store using the freshly allocated temporary StructTemp if
    // needed
    if (!IsRetChanged)
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

  auto ResultType = BaseValue->GetType();

  // case when a pointer is tha base and not an array
  if (ResultType.IsPTR() && !ResultType.IsArray()) {
    // if the base value is on the stack or a global variable
    if (dynamic_cast<StackAllocationInstruction *>(BaseValue) != nullptr ||
        dynamic_cast<GlobalVariable *>(BaseValue) != nullptr) {
      // then load it in first
      BaseValue = IRF->CreateLD(ResultType, BaseValue);
      // since we loaded it in, therefore the result indirection level decreased
      // by one
      ResultType.DecrementPointerLevel();
    }
  } else {
    ResultType.ReduceDimension();
    if (ResultType.GetPointerLevel() == 0)
      ResultType.IncrementPointerLevel();
  }

  auto GEP = IRF->CreateGEP(ResultType, BaseValue, IndexValue);

  if (!GetLValueness() && ResultType.GetDimensions().size() == 0)
    return IRF->CreateLD(ResultType, GEP);

  return GEP;
}

Value *ImplicitCastExpression::IRCodegen(IRFactory *IRF) {
  auto SourceTypeVariant = CastableExpression->GetResultType().GetTypeVariant();
  auto DestTypeVariant = GetResultType().GetTypeVariant();

  // If its an array to pointer decay
  // Note: its only allowed if the expression is a ReferenceExpression
  // TODO: Investigate whether other types of expressions should be allowed
  if (CastableExpression->GetResultType().IsArray() &&
      GetResultType().IsPointerType()) {
    assert(SourceTypeVariant == DestTypeVariant);

    auto RefExp = dynamic_cast<ReferenceExpression *>(CastableExpression.get());

    // StringLiteral case
    if (RefExp == nullptr) {
      auto StrLitExpr = dynamic_cast<StringLiteralExpression *>(CastableExpression.get());
      assert(StrLitExpr && "It must be either a reference or a string literal");

      return StrLitExpr->IRCodegen(IRF);
    }

    auto Referee = RefExp->GetIdentifier();
    auto Res = IRF->GetSymbolValue(Referee);
    if (!Res)
      Res = IRF->GetGlobalVar(Referee);
    assert(Res);

    auto Gep = IRF->CreateGEP(GetIRTypeFromASTType(GetResultType()), Res,
                              IRF->GetConstant((uint64_t)0));

    return Gep;
  }

  auto Val = CastableExpression->IRCodegen(IRF);

  // in case if the expression to be cast is a constant, then no need to do
  // truncation or sign extension, but just masking down the appropriate bits to
  // fit into the desired type
  if (Val->IsConstant()) {
    uint64_t DestBitSize = GetIRTypeFromASTType(GetResultType())
                               .GetByteSize(IRF->GetTargetMachine()) *
                           8;
    uint64_t mask = ~0ull; // full 1s in binary

    // if the bit size is less than 64, then full 1s mask would be wrong, instead
    // we need one which last @DestBitSize bit is 1 and others are 0
    // example: DestBitSize = 16 -> mask = 0x000000000000ffff
    if (DestBitSize < 64)
      mask = (1ull << DestBitSize) - 1;

    auto val = ((Constant *)Val)->GetIntValue() & mask;

    return IRF->GetConstant(val, DestBitSize);
  }

  if (Type::OnlySigndnessDifference(SourceTypeVariant, DestTypeVariant))
    return Val;

  // TODO: simplify this mess
  switch (SourceTypeVariant) {
  case Type::Char: {
    if (DestTypeVariant == Type::Int)
      return IRF->CreateSEXT(Val, 32);
    if (DestTypeVariant == Type::UnsignedInt)
      return IRF->CreateZEXT(Val, 32);
    if (DestTypeVariant == Type::Long || DestTypeVariant == Type::LongLong)
      return IRF->CreateSEXT(Val, 64);
    if (DestTypeVariant == Type::UnsignedLong ||
        DestTypeVariant == Type::UnsignedLongLong)
      return IRF->CreateZEXT(Val, 64);
    assert(!"Invalid conversion.");
  }
  case Type::UnsignedChar: {
    if (DestTypeVariant == Type::Int || DestTypeVariant == Type::UnsignedInt)
      return IRF->CreateZEXT(Val, 32);
    if (DestTypeVariant == Type::Long || DestTypeVariant == Type::LongLong ||
        DestTypeVariant == Type::UnsignedLong ||
        DestTypeVariant == Type::UnsignedLongLong)
      return IRF->CreateZEXT(Val, 64);
    assert(!"Invalid conversion.");
  }
  case Type::Short: {
    if ((DestTypeVariant == Type::Char ||
         DestTypeVariant == Type::UnsignedChar))
      return IRF->CreateTRUNC(Val, 8);
    if (DestTypeVariant == Type::Int)
      return IRF->CreateSEXT(Val, 32);
    if (DestTypeVariant == Type::UnsignedInt)
      return IRF->CreateZEXT(Val, 32);
    if (DestTypeVariant == Type::Long || DestTypeVariant == Type::LongLong)
      return IRF->CreateSEXT(Val, 64);
    if (DestTypeVariant == Type::UnsignedLong ||
        DestTypeVariant == Type::UnsignedLongLong)
      return IRF->CreateZEXT(Val, 64);
    assert(!"Invalid conversion.");
  }
  case Type::UnsignedShort: {
    if ((DestTypeVariant == Type::Char ||
         DestTypeVariant == Type::UnsignedChar))
      return IRF->CreateTRUNC(Val, 8);
    if (DestTypeVariant == Type::Int || DestTypeVariant == Type::UnsignedInt)
      return IRF->CreateZEXT(Val, 32);
    if (DestTypeVariant == Type::Long || DestTypeVariant == Type::LongLong ||
        DestTypeVariant == Type::UnsignedLong ||
        DestTypeVariant == Type::UnsignedLongLong)
      return IRF->CreateZEXT(Val, 64);
    assert(!"Invalid conversion.");
  }
  case Type::Int: {
    if (DestTypeVariant == Type::Double)
      return IRF->CreateITOF(Val, 32);
    if ((DestTypeVariant == Type::Char ||
         DestTypeVariant == Type::UnsignedChar))
      return IRF->CreateTRUNC(Val, 8);
    if (DestTypeVariant == Type::Long || DestTypeVariant == Type::LongLong)
      return IRF->CreateSEXT(Val, 64);
    if (DestTypeVariant == Type::UnsignedLong ||
        DestTypeVariant == Type::UnsignedLongLong)
      return IRF->CreateZEXT(Val, 64);
    assert(!"Invalid conversion.");
  }
  case Type::UnsignedInt: {
    if (DestTypeVariant == Type::Double)
      return IRF->CreateITOF(Val, 32);
    if ((DestTypeVariant == Type::Char ||
         DestTypeVariant == Type::UnsignedChar))
      return IRF->CreateTRUNC(Val, 8);
    if (DestTypeVariant == Type::Long || DestTypeVariant == Type::LongLong ||
        DestTypeVariant == Type::UnsignedLong ||
        DestTypeVariant == Type::UnsignedLongLong)
      return IRF->CreateZEXT(Val, 64);
    assert(!"Invalid conversion.");
  }
  case Type::Long:
  case Type::LongLong: {
    if ((DestTypeVariant == Type::Char ||
         DestTypeVariant == Type::UnsignedChar))
      return IRF->CreateTRUNC(Val, 8);
    if ((DestTypeVariant == Type::Int || DestTypeVariant == Type::UnsignedInt))
      return IRF->CreateTRUNC(Val, 32);
    assert(!"Invalid conversion.");
  }
  case Type::UnsignedLong:
  case Type::UnsignedLongLong: {
    if ((DestTypeVariant == Type::Char ||
         DestTypeVariant == Type::UnsignedChar))
      return IRF->CreateTRUNC(Val, 8);
    if ((DestTypeVariant == Type::Int || DestTypeVariant == Type::UnsignedInt))
      return IRF->CreateTRUNC(Val, 32);
    assert(!"Invalid conversion.");
  }
  case Type::Double: {
    if (DestTypeVariant == Type::Int)
      return IRF->CreateFTOI(Val, 64);
    assert(!"Invalid conversion.");
  }
  default:
    assert(!"Invalid conversion.");
  }

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

  auto BaseType = BaseValue->GetType();
  while (BaseType.GetPointerLevel() > 1) {
    BaseValue = IRF->CreateLD(BaseType, BaseValue);
    BaseType = BaseValue->GetType();
  }

  auto GEP = IRF->CreateGEP(ResultType, BaseValue, IndexValue);

  if (GetLValueness())
    return GEP;

  auto ResultIRType = GetIRTypeFromASTType(this->GetResultType());

  return IRF->CreateLD(ResultIRType, GEP);
}

Value *StructInitExpression::IRCodegen(IRFactory *IRF) {
  // allocate stack for the struct first
  auto IRResultType = GetIRTypeFromASTType(ResultType);
  // TODO: make sure the name will be unique
  auto StructTemp = IRF->CreateSA(ResultType.GetName() + ".temp", IRResultType);

  unsigned LoopIdx = 0;
  for (auto &InitExpr : InitValues) {
    auto InitExprCode = InitExpr->IRCodegen(IRF);
    auto MemberIdx = MemberOrdering[LoopIdx];

    auto ResultType = IRResultType.GetMemberTypes()[MemberIdx];
    ResultType.IncrementPointerLevel();

    auto MemberPtr = IRF->CreateGEP(ResultType, StructTemp,
                                    IRF->GetConstant((uint64_t)MemberIdx));
    IRF->CreateSTR(InitExprCode, MemberPtr);
    LoopIdx++;
  }

  return StructTemp;
}

Value *UnaryExpression::IRCodegen(IRFactory *IRF) {
  Value *E = nullptr;

  if (GetOperationKind() != ADDRESS && GetOperationKind() != MINUS &&
      GetOperationKind() != SIZEOF)
    E = Expr->IRCodegen(IRF);

  switch (GetOperationKind()) {
  case ADDRESS: {
    auto RefExp = dynamic_cast<ReferenceExpression *>(Expr.get());
    assert(RefExp);
    auto Referee = RefExp->GetIdentifier();
    auto Res = IRF->GetSymbolValue(Referee);
    if (!Res) {
      Res = IRF->GetGlobalVar(Referee);
      Res->GetTypeRef().IncrementPointerLevel();
    }
    return Res;
  }
  case DEREF: {
    auto ResultType = E->GetType();
    return IRF->CreateLD(ResultType, E);
  }
  case NOT: {
    // goal IR:
    //    # E generated here
    //    sa $result
    //    str [$result], 0
    //    cmp.eq $c1, $E, 0
    //    br $c1, <end>
    // <true>
    //    str [$result], 1
    //    j <end>
    // <end>
    const auto FuncPtr = IRF->GetCurrentFunction();

    auto TrueBB = std::make_unique<BasicBlock>("not_true", FuncPtr);
    auto FinalBB = std::make_unique<BasicBlock>("not_final", FuncPtr);

    // LHS Test
    auto Result = IRF->CreateSA("result", IRType::CreateBool());
    IRF->CreateSTR(IRF->GetConstant((uint64_t)1), Result);

    // if L was a compare instruction then just revert its relation
    if (auto LCMP = dynamic_cast<CompareInstruction *>(E); LCMP != nullptr) {
      LCMP->InvertRelation();
      IRF->CreateBR(E, FinalBB.get());
    } else {
      auto LHSTest = IRF->CreateCMP(CompareInstruction::EQ, E,
                                    IRF->GetConstant((uint64_t)0));
      IRF->CreateBR(LHSTest, FinalBB.get());
    }

    // TRUE
    IRF->InsertBB(std::move(TrueBB));
    IRF->CreateSTR(IRF->GetConstant((uint64_t)0), Result);
    IRF->CreateJUMP(FinalBB.get());

    IRF->InsertBB(std::move(FinalBB));

    // the result seems to be always an rvalue so loading it also
    return IRF->CreateLD(IRType::CreateBool(), Result);
  }
  case MINUS: {
    if (auto ConstE = dynamic_cast<IntegerLiteralExpression *>(Expr.get());
        ConstE != nullptr) {
      ConstE->SetValue(-ConstE->GetSIntValue());
      return Expr->IRCodegen(IRF);
    }

    E = Expr->IRCodegen(IRF);
    return IRF->CreateSUB(IRF->GetConstant((uint64_t)0), E);
  }
  case POST_DECREMENT:
  case POST_INCREMENT: {
    // make the assumption that the expression E is an LValue which means
    // its basically a pointer, so it requires a load first for addition to work
    auto LoadedValType = E->GetTypeRef();
    LoadedValType.DecrementPointerLevel();
    auto LoadedExpr = IRF->CreateLD(LoadedValType, E);

    Instruction *AddSub;
    if (GetOperationKind() == POST_INCREMENT)
      AddSub = IRF->CreateADD(LoadedExpr, IRF->GetConstant((uint64_t)1));
    else
      AddSub = IRF->CreateSUB(LoadedExpr, IRF->GetConstant((uint64_t)1));

    IRF->CreateSTR(AddSub, E);
    return LoadedExpr;
  }
  case PRE_DECREMENT:
  case PRE_INCREMENT: {
    // make the assumption that the expression E is an LValue which means
    // its basically a pointer, so it requires a load first for addition to work
    auto LoadedValType = E->GetTypeRef();
    LoadedValType.DecrementPointerLevel();
    auto LoadedExpr = IRF->CreateLD(LoadedValType, E);

    Instruction *AddSub;
    if (GetOperationKind() == PRE_INCREMENT)
      AddSub = IRF->CreateADD(LoadedExpr, IRF->GetConstant((uint64_t)1));
    else
      AddSub = IRF->CreateSUB(LoadedExpr, IRF->GetConstant((uint64_t)1));

    IRF->CreateSTR(AddSub, E);
    return AddSub;
  }
  case SIZEOF: {
    uint64_t size = 0;
    Type TypeToBeExamined = ResultType;

    // if there was an expression used with sizeof, use thats result type
    // instead
    if (Expr)
      TypeToBeExamined = Expr->GetResultType();

    size = GetIRTypeFromASTType(TypeToBeExamined)
               .GetByteSize(IRF->GetTargetMachine());

    assert(size != 0 && "sizeof should not result in 0");

    return IRF->GetConstant(size);
  }
  default:
    assert(!"Unimplemented");
  }

  return nullptr;
}

Value *BinaryExpression::IRCodegen(IRFactory *IRF) {
  // TODO: simplify this, specially in case if there are actually multiple
  // logical operations like "a > 0 && a < 10 && a != 5"
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

    // LHS Test
    auto Result = IRF->CreateSA("result", IRType::CreateBool());
    IRF->CreateSTR(IRF->GetConstant((uint64_t)0), Result);

    auto L = Left->IRCodegen(IRF);

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

    // the result seems to be always an rvalue so loading it also
    return IRF->CreateLD(IRType::CreateBool(), Result);
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
      GetOperationKind() == MUL_ASSIGN || GetOperationKind() == DIV_ASSIGN ||
      GetOperationKind() == MOD_ASSIGN || GetOperationKind() == AND_ASSIGN ||
      GetOperationKind() == OR_ASSIGN || GetOperationKind() == XOR_ASSIGN ||
      GetOperationKind() == LSL_ASSIGN || GetOperationKind() == LSR_ASSIGN) {
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
      case MOD_ASSIGN:
        // TODO: what about MODU?
        OperationResult = IRF->CreateMOD(L, R);
        break;
      case AND_ASSIGN:
        OperationResult = IRF->CreateAND(L, R);
        break;
      case OR_ASSIGN:
        OperationResult = IRF->CreateOR(L, R);
        break;
      case XOR_ASSIGN:
        OperationResult = IRF->CreateXOR(L, R);
        break;
      case LSL_ASSIGN:
        OperationResult = IRF->CreateLSL(L, R);
        break;
      case LSR_ASSIGN:
        OperationResult = IRF->CreateLSR(L, R);
        break;
      default:
        assert(!"Unreachable");
      }
      // TODO: Revisit this. Its not necessary guaranteed that it will be a load
      // for now it seems fine
      auto Load = dynamic_cast<LoadInstruction *>(L);
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
    case XOR:
    case AND:
    case OR:
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

  if (L->IsConstant())
    L = IRF->CreateMOV(L, R->GetBitWidth());

  switch (GetOperationKind()) {
  case LSL:
    return IRF->CreateLSL(L, R);
  case LSR:
    return IRF->CreateLSR(L, R);
  case ADD:
    return IRF->CreateADD(L, R);
  case SUB:
    return IRF->CreateSUB(L, R);
  case MUL:
    return IRF->CreateMUL(L, R);
  case DIV:
    return IRF->CreateDIV(L, R);
  case DIVU:
    return IRF->CreateDIVU(L, R);
  case MOD:
    return IRF->CreateMOD(L, R);
  case MODU:
    return IRF->CreateMODU(L, R);
  case AND:
    return IRF->CreateAND(L, R);
  case OR:
    return IRF->CreateOR(L, R);
  case XOR:
    return IRF->CreateXOR(L, R);
  case EQ:
    return IRF->CreateCMP(CompareInstruction::EQ, L, R);
  case LT:
    return IRF->CreateCMP(CompareInstruction::LT, L, R);
  case GT:
    return IRF->CreateCMP(CompareInstruction::GT, L, R);
  case NE:
    return IRF->CreateCMP(CompareInstruction::NE, L, R);
  case GE:
    return IRF->CreateCMP(CompareInstruction::GE, L, R);
  case LE:
    return IRF->CreateCMP(CompareInstruction::LE, L, R);
  default:
    assert(!"Unhandled binary instruction type");
    break;
  }
}

Value *TernaryExpression::IRCodegen(IRFactory *IRF) {
  // goal IR:
  //    # Condition generated here
  //    sa $result
  //    cmp.eq $c1, $Condition, 0
  //    br $c1, <false>
  // <true>
  //    # ExprIfTrue generated here
  //    str [$result], $ExprIfTrue
  //    j <end>
  // <false>
  //    # ExprIfFalse generated here
  //    str [$result], ExprIfFalse
  //    j <end>
  // <end>

  const auto FuncPtr = IRF->GetCurrentFunction();

  auto TrueBB = std::make_unique<BasicBlock>("ternary_true", FuncPtr);
  auto FalseBB = std::make_unique<BasicBlock>("ternary_false", FuncPtr);
  auto FinalBB = std::make_unique<BasicBlock>("ternary_end", FuncPtr);

  auto C = Condition->IRCodegen(IRF);

  // Condition Test

  // if L was a compare instruction then just revert its relation
  if (auto LCMP = dynamic_cast<CompareInstruction *>(C); LCMP != nullptr) {
    LCMP->InvertRelation();
    IRF->CreateBR(C, FalseBB.get());
  } else {
    auto LHSTest = IRF->CreateCMP(CompareInstruction::EQ, C,
                                  IRF->GetConstant((uint64_t)0));
    IRF->CreateBR(LHSTest, FalseBB.get());
  }

  // TRUE
  IRF->InsertBB(std::move(TrueBB));
  auto TrueExpr = ExprIfTrue->IRCodegen(IRF);
  auto Result = IRF->CreateSA("result", TrueExpr->GetType());
  IRF->CreateSTR(TrueExpr, Result);
  IRF->CreateJUMP(FinalBB.get());

  // FALSE
  IRF->InsertBB(std::move(FalseBB));
  IRF->CreateSTR(ExprIfFalse->IRCodegen(IRF), Result);
  IRF->CreateJUMP(FinalBB.get());

  IRF->InsertBB(std::move(FinalBB));
  return IRF->CreateLD(Result->GetType(), Result);
}

Value *IntegerLiteralExpression::IRCodegen(IRFactory *IRF) {
  return IRF->GetConstant(IntValue);
}

Value *FloatLiteralExpression::IRCodegen(IRFactory *IRF) {
  return IRF->GetConstant(FPValue);
}

Value *StringLiteralExpression::IRCodegen(IRFactory *IRF) {
  static unsigned counter = 0; // used to create unique names
  std::string Name = ".L.str" + std::to_string(counter++);
  auto Type = GetIRTypeFromASTType(ResultType);
  // the global variable is now a pointer to the data
  Type.IncrementPointerLevel();
  // create a global variable for the string literal with the label Name
  auto GV = IRF->CreateGlobalVar(Name, Type, StringValue);
  IRF->AddGlobalVariable(GV);
  return GV;
}

Value *TranslationUnit::IRCodegen(IRFactory *IRF) {
  for (auto &Declaration : Declarations) {
    IRF->SetGlobalScope();
    if (auto Decl = Declaration->IRCodegen(IRF); Decl != nullptr)
      IRF->AddGlobalVariable(Decl);
  }
  return nullptr;
}
