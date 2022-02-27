#include "ASTPrint.hpp"
#include "AST.hpp"


static void PrintImpl(const char *str, unsigned tab = 0, bool newline = false) {
  for (size_t i = 0; i < tab; i++)
    std::cout << " ";
  std::cout << str;
  if (newline)
    std::cout << std::endl;
}

static void Print(const char *str, unsigned tab = 0) { PrintImpl(str, tab); }

static void PrintLn(const char *str, unsigned tab = 0) {
  PrintImpl(str, tab, true);
}

ASTPrint::ASTPrint() {}

void ASTPrint::VisitVariableDeclaration(const VariableDeclaration *node) {
  Print("VariableDeclaration ", tab);
  auto TypeStr = "'" + node->GetType().ToString() + "' ";
  Print(TypeStr.c_str());
  auto NameStr = "'" + node->GetName() + "'";
  PrintLn(NameStr.c_str());
  tab += 2;
  if (node->GetInitExpr())
    node->GetInitExpr()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitMemberDeclaration(const MemberDeclaration *node) {
  Print("MemberDeclaration ", tab);
  auto TypeStr = "'" + node->GetType().ToString() + "' ";
  Print(TypeStr.c_str());
  auto NameStr = "'" + node->GetName() + "'";
  PrintLn(NameStr.c_str());
}

void ASTPrint::VisitStructDeclaration(const StructDeclaration *node) {
  Print("StructDeclaration '", tab);
  Print(node->GetName().c_str());
  PrintLn("' ");
  for (auto &M : node->GetMembers())
    M->Accept(this);
}

void ASTPrint::VisitEnumDeclaration(const EnumDeclaration *node) {
  std::string Str = "EnumDeclaration '";
  Str += node->GetBaseType().ToString() + "'";
  PrintLn(Str.c_str(), tab);
  Str.clear();
  Str = "Enumerators ";
  unsigned LoopCounter = 0;

  for (auto &[Enum, Val] : node->GetEnumerators()) {
    Str += "'" + Enum + "'";
    Str += " = " + std::to_string(Val);
    if (++LoopCounter < node->GetEnumerators().size())
      Str += ", ";
  }
  PrintLn(Str.c_str(), tab + 2);
}

void ASTPrint::VisitCompoundStatement(const CompoundStatement *node) {
  PrintLn("CompoundStatement", tab);
  tab += 2;
  for (size_t i = 0; i < node->GetStatements().size(); i++)
    node->GetStatements()[i]->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitExpressionStatement(const ExpressionStatement *node) {
  PrintLn("ExpressionStatement", tab);
  tab += 2;
  node->GetExpression()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitIfStatement(const IfStatement *node) {
  PrintLn("IfStatement", tab);
  tab += 2;
  node->GetCondition()->Accept(this);
  node->GetIfBody()->Accept(this);
  if (node->GetElseBody())
    node->GetElseBody()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitSwitchStatement(const SwitchStatement *node) {
  PrintLn("SwitchStatement", tab);
  tab += 2;
  node->GetCondition()->Accept(this);

  for (auto &[CaseConst, CaseBody] : node->GetCaseBodies()) {
    std::string Str = "Case '" + std::to_string(CaseConst) + "'";
    PrintLn(Str.c_str(), tab);
    Str.clear();
    tab += 2;
    for (auto &CaseStatement : CaseBody)
      CaseStatement->Accept(this);
    tab -= 2;
  }

  if (node->GetDefaultBody().size() > 0)
    PrintLn("DefaultCase", tab);
  tab += 2;
  for (auto &DefaultStatement : node->GetDefaultBody())
    DefaultStatement->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitWhileStatement(const WhileStatement *node) {
  PrintLn("WhileStatement", tab);
  tab += 2;
  node->GetCondition()->Accept(this);
  node->GetBody()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitDoWhileStatement(const DoWhileStatement *node) {
  PrintLn("DoWhileStatement", tab);
  tab += 2;
  node->GetBody()->Accept(this);
  node->GetCondition()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitForStatement(const ForStatement *node) {
  PrintLn("ForStatement", tab);
  tab += 2;

  if (node->GetInit())
    node->GetInit()->Accept(this);
  else {
    auto &VarDecls = node->GetVarDecls();
    for (auto &VarDecl : VarDecls)
      VarDecl->Accept(this);
  }

  node->GetCondition()->Accept(this);
  node->GetIncrement()->Accept(this);
  node->GetBody()->Accept(this);

  tab -= 2;
}

void ASTPrint::VisitReturnStatement(const ReturnStatement *node) {
  PrintLn("ReturnStatement", tab);
  tab += 2;
  if (node->GetRetVal())
    node->GetRetVal()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitBreakStatement(const BreakStatement *node) {
  PrintLn("BreakStatement", tab);
}

void ASTPrint::VisitContinueStatement(const ContinueStatement *node) {
  PrintLn("ContinueStatement", tab);
}

void ASTPrint::VisitFunctionParameterDeclaration(
    const FunctionParameterDeclaration *node) {
  Print("FunctionParameterDeclaration ", tab);
  auto TypeStr = "'" + node->GetType().ToString() + "' ";
  Print(TypeStr.c_str());
  auto NameStr = "'" + node->GetName() + "'";
  PrintLn(NameStr.c_str());
}

void ASTPrint::VisitFunctionDeclaration(const FunctionDeclaration *node) {
  Print("FunctionDeclaration ", tab);
  auto TypeStr = "'" + node->GetType().ToString() + "' ";
  Print(TypeStr.c_str());
  auto NameStr = "'" + node->GetName() + "'";
  PrintLn(NameStr.c_str());

  tab += 2;

  for (size_t i = 0; i < node->GetArguments().size(); i++)
    node->GetArguments()[i]->Accept(this);
  if (node->GetBody())
    node->GetBody()->Accept(this);

  tab -= 2;
}

void ASTPrint::VisitBinaryExpression(const BinaryExpression *node) {
  Print("BinaryExpression ", tab);
  auto Str = "'" + node->GetResultType().ToString() + "' ";
  Str += "'" + node->GetOperation().GetString() + "'";
  PrintLn(Str.c_str());

  tab += 2;
  node->GetLeftExpr()->Accept(this);
  node->GetRightExpr()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitTernaryExpression(const TernaryExpression *node) {
  Print("TernaryExpression ", tab);
  auto Str = "'" + node->GetResultType().ToString() + "' ";
  PrintLn(Str.c_str());

  tab += 2;
  node->GetCondition()->Accept(this);
  node->GetExprIfTrue()->Accept(this);
  node->GetExprIfFalse()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitStructMemberReference(const StructMemberReference *node) {
  Print("StructMemberReference ", tab);
  auto Str = "'" + node->GetResultType().ToString() + "' ";
  Str += "'." + node->GetMemberId() + "'";
  PrintLn(Str.c_str());
  tab += 2;
  node->GetExpr()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitStructInitExpression(const StructInitExpression *node) {
  Print("StructInitExpression ", tab);
  auto Str = "'" + node->GetResultType().ToString() + "' ";
  PrintLn(Str.c_str());

  tab += 2;
  for (auto &InitValue : node->GetInitList())
    InitValue->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitUnaryExpression(const UnaryExpression *node) {
  Print("UnaryExpression ", tab);
  std::string Str;
  if (node->GetOperationKind() == UnaryExpression::SIZEOF)
    Str += "'" + Type(Type::UnsignedInt).ToString() + "' ";
  else
    Str += "'" + node->GetResultType().ToString() + "' ";

  Str += "'" + node->GetOperation().GetString() + "'";

  if (!node->GetExpr())
    Str += " '" + node->GetResultType().ToString() + "'";

  PrintLn(Str.c_str());

  tab += 2;
  if (node->GetExpr())
    node->GetExpr()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitCallExpression(const CallExpression *node) {
  Print("CallExpression ", tab);
  auto Str = "'" + node->GetResultType().ToString() + "' ";
  Str += "'" + node->GetName() + "'";
  PrintLn(Str.c_str());

  tab += 2;
  for (size_t i = 0; i < node->GetArguments().size(); i++)
    node->GetArguments()[i]->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitReferenceExpression(const ReferenceExpression *node) {
  Print("ReferenceExpression ", tab);
  auto Str = "'" + node->GetResultType().ToString() + "' ";
  Str += "'" + node->GetIdentifier() + "'";
  PrintLn(Str.c_str());
}

void ASTPrint::VisitIntegerLiteralExpression(
    const IntegerLiteralExpression *node) {
  Print("IntegerLiteralExpression ", tab);
  auto TyStr = "'" + node->GetResultType().ToString() + "' ";
  Print(TyStr.c_str());
  auto ValStr = "'" + std::to_string(node->GetSIntValue()) + "'";
  PrintLn(ValStr.c_str());
}

void ASTPrint::VisitFloatLiteralExpression(const FloatLiteralExpression *node) {
  Print("FloatLiteralExpression ", tab);
  auto TyStr = "'" + node->GetResultType().ToString() + "' ";
  Print(TyStr.c_str());
  auto ValStr = "'" + std::to_string(node->GetValue()) + "'";
  PrintLn(ValStr.c_str());
}

void ASTPrint::VisitStringLiteralExpression(const StringLiteralExpression *node) {
  Print("StringLiteralExpression ", tab);
  auto TyStr = "'" + node->GetResultType().ToString() + "' ";
  Print(TyStr.c_str());
  auto ValStr = "'" + node->GetValue() + "'";
  PrintLn(ValStr.c_str());
}

void ASTPrint::VisitArrayExpression(const ArrayExpression *node) {
  Print("ArrayExpression ", tab);
  auto Str = "'" + node->GetResultType().ToString() + "' ";
  PrintLn(Str.c_str());

  tab += 2;
  node->GetBaseExpression()->Accept(this);
  node->GetIndexExpression()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitImplicitCastExpression(const ImplicitCastExpression *node) {
  Print(node->IsExplicit() ? "CastExpression" : "ImplicitCastExpression", tab);
  auto Str = " '" + node->GetResultType().ToString() + "'";
  PrintLn(Str.c_str());

  tab += 2;
  node->GetCastableExpression()->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitInitializerListExpression(
    const InitializerListExpression *node) {
  PrintLn("InitializerListExpression", tab);

  tab += 2;
  for (auto &E : node->GetExprList())
    E->Accept(this);
  tab -= 2;
}

void ASTPrint::VisitTranslationUnit(const TranslationUnit *node) {
  PrintLn("TranslationUnit", tab);

  tab += 2;
  for (size_t i = 0; i < node->GetDeclarations().size(); i++)
    node->GetDeclarations()[i]->Accept(this);
  tab -= 2;

  PrintLn("");
}