#include "Semantics.hpp"
#include "AST.hpp"
#include <tuple>

//=--------------------------------------------------------------------------=//
//=--------------------------- Helper functions -----------------------------=//
//=--------------------------------------------------------------------------=//

void Semantics::InsertToSymTable(const Token &SymName, const Type &SymType,
                                 const bool ToGlobal, ValueType SymValue) {

  SymbolTableStack::Entry SymEntry(SymName, SymType, SymValue);
  auto SymNameStr = SymName.GetString();

  std::optional<SymbolTableStack::Entry> ExistingEntry =
      ToGlobal ? SymbolTables.ContainsInGlobalScope(SymNameStr)
               : SymbolTables.ContainsInCurrentScope(SymNameStr);

  bool IsRedefinition = ExistingEntry.has_value();

  // If the existing definition is just a prototype, then it is not an error
  if (auto FuncDecl = GetFuncDecl(SymNameStr);
      FuncDecl != nullptr && FuncDecl->GetBody() == nullptr)
    IsRedefinition = false;

  // Check if it is already defined in the current scope
  if (IsRedefinition) {
    std::string Msg = "redefinition of '" + SymNameStr + "'";
    ErrorLog.AddError(Msg, SymName);

    Msg = "previous definition was here";
    ErrorLog.AddNote(Msg, std::get<0>(ExistingEntry.value()));
  } else if (ToGlobal)
    SymbolTables.InsertGlobalEntry(SymEntry);
  else
    SymbolTables.InsertEntry(SymEntry);
}

const FunctionDeclaration *Semantics::GetFuncDecl(const std::string &FuncName) {
  for (auto FuncDecl : FuncDeclList)
    if (FuncDecl->GetName() == FuncName)
      return FuncDecl;

  return nullptr;
}

static Expression *GetExprIgnoreImplicitCast(Expression *Expr) {
  Expression *CastedExpr = Expr;

  while (auto ICE = dynamic_cast<ImplicitCastExpression *>(CastedExpr))
    CastedExpr = ICE->GetCastableExpression().get();

  return CastedExpr;
}

//=--------------------------------------------------------------------------=//
//=----------------------------- Sema functions -----------------------------=//
//=--------------------------------------------------------------------------=//

void Semantics::VisitVariableDeclaration(const VariableDeclaration *node) {
  auto VarName = node->GetName();

  InsertToSymTable(node->GetNameToken(), node->GetType());

  if (node->GetInitExpr())
    node->GetInitExpr()->Accept(this);
}

void Semantics::VisitMemberDeclaration(const MemberDeclaration *node) {}

void Semantics::VisitStructDeclaration(const StructDeclaration *node) {
  // Register the incomplete type
  UserDefinedTypes[node->GetName()] = {node->GetType(), {}};

  for (auto &M : node->GetMembers())
    M->Accept(this);

  // Register the fully defined type
  std::vector<Token> StructMemberIdentifiers;
  for (auto &Member : node->GetMembers())
    StructMemberIdentifiers.push_back(Member->GetNameToken());

  UserDefinedTypes[node->GetName()] = {node->GetType(),
                                       {StructMemberIdentifiers}};
}

void Semantics::VisitEnumDeclaration(const EnumDeclaration *node) {}

void Semantics::VisitCompoundStatement(const CompoundStatement *node) {
  // Open new scope
  SymbolTables.PushSymTable();

  for (const auto &Stmt : node->GetStatements()) {
    Stmt->Accept(this);
  }

  // Closing the scope
  SymbolTables.PopSymTable();
}

void Semantics::VisitExpressionStatement(const ExpressionStatement *node) {
  if (node->GetExpression() != nullptr)
    node->GetExpression()->Accept(this);
}

void Semantics::VisitIfStatement(const IfStatement *node) {
  node->GetCondition()->Accept(this);
  node->GetIfBody()->Accept(this);
  if (node->GetElseBody())
    node->GetElseBody()->Accept(this);
}

void Semantics::VisitSwitchStatement(const SwitchStatement *node) {
  node->GetCondition()->Accept(this);

  for (auto &[CaseConst, CaseBody] : node->GetCaseBodies()) {
    // If non const value used as label
    if (!dynamic_cast<IntegerLiteralExpression *>(CaseConst.get())) {
      // TODO: Add somehow line and col number info
      std::string Msg = "case label does not reduce to an integer constant";
      ErrorLog.AddError(Msg);
    }

    for (auto &CaseStatement : CaseBody)
      CaseStatement->Accept(this);
  }

  if (!node->GetDefaultBody().empty())
    for (auto &DefaultStatement : node->GetDefaultBody())
      DefaultStatement->Accept(this);
}

void Semantics::VisitWhileStatement(const WhileStatement *node) {
  node->GetCondition()->Accept(this);
  node->GetBody()->Accept(this);
}

void Semantics::VisitDoWhileStatement(const DoWhileStatement *node) {
  node->GetBody()->Accept(this);
  node->GetCondition()->Accept(this);
}

void Semantics::VisitForStatement(const ForStatement *node) {
  // Open new scope
  SymbolTables.PushSymTable();

  if (node->GetInit())
    node->GetInit()->Accept(this);
  else {
    auto &VarDecls = node->GetVarDecls();
    for (auto &VarDecl : VarDecls)
      VarDecl->Accept(this);
  }

  if (node->GetCondition())
    node->GetCondition()->Accept(this);
  if (node->GetIncrement())
    node->GetIncrement()->Accept(this);
  node->GetBody()->Accept(this);

  // Closing the scope
  SymbolTables.PopSymTable();
}

void Semantics::VisitReturnStatement(const ReturnStatement *node) {
  if (node->GetRetVal())
    node->GetRetVal()->Accept(this);
}

void Semantics::VisitBreakStatement(const BreakStatement *node) {}

void Semantics::VisitContinueStatement(const ContinueStatement *node) {}

void Semantics::VisitFunctionParameterDeclaration(
    const FunctionParameterDeclaration *node) {
  InsertToSymTable(node->GetNameToken(), node->GetType());
}

void Semantics::VisitFunctionDeclaration(const FunctionDeclaration *node) {
  auto FuncName = node->GetName();

  InsertToSymTable(node->GetNameToken(), node->GetType(), true);

  // Opening a new scope for the function
  SymbolTables.PushSymTable();

  for (const auto &Arg : node->GetArguments())
    Arg->Accept(this);

  // In case of a function definition
  if (node->GetBody()) {
    auto &ParamList = node->GetArguments();

    for (auto &Param : ParamList)
      if (Param->GetTypeRef().IsVoid()) {
        std::string Msg =
            "parameter ('" + Param->GetName() + "') has incomplete type";
        ErrorLog.AddError(Msg, Param->GetNameToken());
      }
      // Validate that a parameter has a name as well. Function prototypes are
      // allowed to omit it, but since this function declaration has a body,
      // therefore it is a function definition as well. Parameters have to be
      // named in this case.
      else if (Param->GetName().empty()) {
        std::string Msg = "parameter name omitted";
        // TODO: Using the function name token to report the error since
        // type does not hold the token which defined it. Maybe add the
        // defining token to the type.
        ErrorLog.AddError(Msg, node->GetNameToken());
      }

    node->GetBody()->Accept(this);
  }

  // Closing the function scope
  SymbolTables.PopSymTable();

  // Adding this function declaration to the already processed ones
  FuncDeclList.push_back(node);
}

void Semantics::VisitBinaryExpression(const BinaryExpression *node) {
  // Check if the left-hand side of the assignment is an LValue
  if (node->IsAssignment() && !node->GetLeftExpr()->GetLValueness()) {
    std::string Msg = "lvalue required as left operand of assignment";
    ErrorLog.AddError(Msg, node->GetOperation());
  }

  const auto &LeftType = node->GetLeftExpr()->GetResultType();
  const auto &RightType = node->GetRightExpr()->GetResultType();

  // Ensure that the left-hand side of the assignment is not a const
  if (node->IsAssignment() && LeftType.IsConst()) {
    std::string Msg = "cannot assign to variable with const-qualified type '" +
                      LeftType.ToString() + "'";
    ErrorLog.AddError(Msg, node->GetOperation());
  }

  // Modulo operator only applicable to integers
  // Shift right-hand operand must be an integer
  if ((node->IsModulo() &&
       (!LeftType.IsIntegerType() || !RightType.IsIntegerType())) ||
      (node->IsShift() && !RightType.IsIntegerType())) {
    std::string Msg = "invalid operands to binary expression ('" +
                      GetExprIgnoreImplicitCast(node->GetLeftExpr().get())
                          ->GetResultType()
                          .ToString() +
                      "' and '" + RightType.ToString() + "')";
    ErrorLog.AddError(Msg, node->GetOperation());
  }

  // Check if the types are matching in an assignment
  if (node->IsAssignment() && LeftType != RightType) {
    std::string Msg = "incompatible types when assigning to type '" +
                      LeftType.ToString() + "' from type '" +
                      RightType.ToString() + "')";
    ErrorLog.AddError(Msg, node->GetOperation());
  }

  node->GetLeftExpr()->Accept(this);
  node->GetRightExpr()->Accept(this);
}

void Semantics::VisitTernaryExpression(const TernaryExpression *node) {
  node->GetCondition()->Accept(this);
  node->GetExprIfTrue()->Accept(this);
  node->GetExprIfFalse()->Accept(this);
}

void Semantics::VisitStructMemberReference(const StructMemberReference *node) {
  // Validate that indeed a struct is the base for the member reference
  if (!node->IsArrow() && !node->GetExpr()->GetResultType().IsStruct()) {
    std::string Msg = "request for member '" + node->GetMemberId() +
                      "' in something not a structure or union";
    ErrorLog.AddError(Msg, node->GetMemberIdToken());
  }

  // Validate that in case of -> operator the base is a pointer
  if (node->IsArrow() && !node->GetExpr()->GetResultType().IsPointerType()) {
    std::string Msg = "invalid type argument of '->' (have '" +
                      node->GetExpr()->GetResultType().ToString() + "')";
    ErrorLog.AddError(Msg, node->GetMemberIdToken());
  }

  // Validate that the accessed member name is exists in the struct
  bool FoundMatch = false;
  const auto StructName = node->GetExpr()->GetResultType().GetName();
  for (auto StructMember : std::get<1>(UserDefinedTypes[StructName])) {
    if (StructMember == node->GetMemberIdToken()) {
      FoundMatch = true;
      break;
    }
  }
  if (!FoundMatch) {
    std::string Msg = "'" +
                      std::get<0>(UserDefinedTypes[StructName]).ToString() +
                      "' has no member named '" + node->GetMemberId() + "'";
    ErrorLog.AddError(Msg, node->GetMemberIdToken());
  }

  node->GetExpr()->Accept(this);
}

void Semantics::VisitStructInitExpression(const StructInitExpression *node) {
  for (auto &InitValue : node->GetInitList())
    InitValue->Accept(this);
}

void Semantics::VisitUnaryExpression(const UnaryExpression *node) {
  // Validate that the dereferenced expression has pointer type
  if (node->GetOperationKind() == UnaryExpression::DEREF &&
      !node->GetExpr()->GetResultType().IsPointerType()) {
    std::string Msg = "invalid type argument of unary '*' (have '" +
                      node->GetExpr()->GetResultType().ToString() + "')";
    ErrorLog.AddError(Msg, node->GetOperation());
  }

  if (node->GetExpr())
    node->GetExpr()->Accept(this);
}

void Semantics::VisitCallExpression(const CallExpression *node) {
  auto CalledFunc = SymbolTables.Contains(node->GetName());

  if (!CalledFunc) {
    std::string Msg =
        "implicit declaration of function '" + node->GetName() + "'";
    ErrorLog.AddWarning(Msg, node->GetNameToken());
  } else {
    // Calling the function with too many argument
    auto &[CalledFuncName, CalledFuncType, _] = CalledFunc.value();
    const auto FuncArgNum = CalledFuncType.GetArgTypes().size();
    const auto CallArgNum = node->GetArguments().size();
    if (!CalledFuncType.HasVarArg() && FuncArgNum != CallArgNum &&
        // Exception case if the function has only one void argument, in which
        // calling it without a parameter is permitted.
        !(FuncArgNum == 1 && CallArgNum == 0 &&
          CalledFuncType.GetArgTypes()[0] == Type(Type::Void))) {
      std::string Num = FuncArgNum > CallArgNum ? "few" : "many";
      std::string Msg =
          "too " + Num + " arguments to function '" + node->GetName() + "'";
      ErrorLog.AddError(Msg, node->GetNameToken());
    }
  }

  for (const auto &Arg : node->GetArguments())
    Arg->Accept(this);
}

void Semantics::VisitReferenceExpression(const ReferenceExpression *node) {
  if (!SymbolTables.Contains(node->GetIdentifier())) {
    std::string Msg = "symbol is undefined '" + node->GetIdentifier() + "'";
    ErrorLog.AddError(Msg, node->GetIdentifierToken());
  }
}

void Semantics::VisitIntegerLiteralExpression(
    const IntegerLiteralExpression *node) {}

void Semantics::VisitFloatLiteralExpression(
    const FloatLiteralExpression *node) {}

void Semantics::VisitStringLiteralExpression(
    const StringLiteralExpression *node) {}

void Semantics::VisitArrayExpression(const ArrayExpression *node) {
  // Check if the left-hand side of the assignment is an LValue
  if (!node->GetBaseExpression()->GetResultType().IsArray() &&
      !node->GetBaseExpression()->GetResultType().IsPointerType()) {
    // TODO: Add somehow line and col number info
    std::string Msg = "subscripted value is not an array nor a pointer";
    ErrorLog.AddError(Msg);
  }

  node->GetBaseExpression()->Accept(this);
  node->GetIndexExpression()->Accept(this);
}

void Semantics::VisitImplicitCastExpression(
    const ImplicitCastExpression *node) {
  node->GetCastableExpression()->Accept(this);
}

void Semantics::VisitInitializerListExpression(
    const InitializerListExpression *node) {
  for (auto &E : node->GetExprList())
    E->Accept(this);
}

void Semantics::VisitTranslationUnit(const TranslationUnit *node) {
  for (const auto &Decl : node->GetDeclarations())
    Decl->Accept(this);
}
