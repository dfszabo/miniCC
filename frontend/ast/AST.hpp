#ifndef AST_HPP
#define AST_HPP

#include "../../middle_end/IR/IRFactory.hpp"
#include "../../middle_end/IR/Value.hpp"
#include "../lexer/Token.hpp"
#include "Type.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

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

class Node {
public:
  virtual ~Node(){};
  virtual void ASTDump(unsigned tab = 0) { PrintLn("Node"); }
  virtual Value *IRCodegen(IRFactory *IRF) {
    assert(!"Must be a child node type");
    return nullptr;
  }
};

class Statement : public Node {
public:
  void ASTDump(unsigned tab = 0) override { PrintLn("Statement", tab); }
};

class Expression : public Node {
public:
  Expression() = default;
  Expression(Type t) : ResultType(std::move(t)) {}
  Expression(Type::VariantKind vk) : ResultType(vk) {}
  Type &GetResultType() { return ResultType; }
  void SetType(Type t) { ResultType = t; }

  void SetLValueness(bool p) { IsLValue = p; }
  bool GetLValueness() { return IsLValue; }

  void ASTDump(unsigned tab = 0) override { PrintLn("Expression", tab); }

protected:
  bool IsLValue = false;
  Type ResultType;
};

class VariableDeclaration : public Statement {
public:
  std::string &GetName() { return Name; }
  void SetName(std::string &s) { Name = s; }

  Type GetType() { return AType; }
  void SetType(Type t) { AType = t; }

  VariableDeclaration(std::string &Name, Type Ty, std::vector<unsigned> Dim)
      : Name(Name), AType(Ty, std::move(Dim)) {}

  VariableDeclaration(std::string &Name, Type Ty) : Name(Name), AType(Ty) {}

  VariableDeclaration() = default;

  void ASTDump(unsigned tab = 0) override {
    Print("VariableDeclaration ", tab);
    auto TypeStr = "'" + AType.ToString() + "' ";
    Print(TypeStr.c_str());
    auto NameStr = "'" + Name + "'";
    PrintLn(NameStr.c_str());
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Name;
  Type AType;
};

class MemberDeclaration : public Statement {
public:
  std::string &GetName() { return Name; }
  void SetName(std::string &s) { Name = s; }

  Type GetType() { return AType; }
  void SetType(Type t) { AType = t; }

  MemberDeclaration(std::string &Name, Type Ty, std::vector<unsigned> Dim)
      : Name(Name), AType(Ty, std::move(Dim)) {}

  MemberDeclaration(std::string &Name, Type Ty) : Name(Name), AType(Ty) {}

  MemberDeclaration() = default;

  void ASTDump(unsigned tab = 0) override {
    Print("MemberDeclaration ", tab);
    auto TypeStr = "'" + AType.ToString() + "' ";
    Print(TypeStr.c_str());
    auto NameStr = "'" + Name + "'";
    PrintLn(NameStr.c_str());
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Name;
  Type AType;
};

class StructDeclaration : public Statement {
public:
  std::string &GetName() { return Name; }
  void SetName(std::string &s) { Name = s; }

  std::vector<std::unique_ptr<MemberDeclaration>> &GetMembers() {
    return Members;
  }
  void SetType(std::vector<std::unique_ptr<MemberDeclaration>> m) {
    Members = std::move(m);
  }

  StructDeclaration(std::string &Name,
                    std::vector<std::unique_ptr<MemberDeclaration>> &M,
                    Type &StructType)
      : Name(Name), Members(std::move(M)), SType(StructType) {}

  StructDeclaration() = default;

  void ASTDump(unsigned tab = 0) override {
    Print("StructDeclaration '", tab);
    Print(Name.c_str());
    PrintLn("' ");
    for (auto &M : Members)
      M->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  Type SType;
  std::string Name;
  std::vector<std::unique_ptr<MemberDeclaration>> Members;
};

class CompoundStatement : public Statement {
  using DeclVec = std::vector<std::unique_ptr<VariableDeclaration>>;
  using StmtVec = std::vector<std::unique_ptr<Statement>>;

public:
  StmtVec &GetStatements() { return Statements; }
  void SetStatements(StmtVec &s) { Statements = std::move(s); }
  void AddStatement(std::unique_ptr<Statement> &s) {
    Statements.push_back(std::move(s));
  }

  DeclVec &GetDeclarations() { return Declarations; }
  void SetDeclarations(DeclVec &d) { Declarations = std::move(d); }
  void AddDeclaration(std::unique_ptr<VariableDeclaration> &d) {
    Declarations.push_back(std::move(d));
  }

  CompoundStatement(DeclVec &Decls, StmtVec &Stats)
      : Declarations(std::move(Decls)), Statements(std::move(Stats)) {}

  CompoundStatement() = delete;

  CompoundStatement(const CompoundStatement &) = delete;
  CompoundStatement &operator=(const CompoundStatement &) = delete;

  CompoundStatement(CompoundStatement &&) = default;

  void ASTDump(unsigned tab = 0) override {
    PrintLn("CompoundStatement", tab);
    for (size_t i = 0; i < Declarations.size(); i++)
      Declarations[i]->ASTDump(tab + 2);
    for (size_t i = 0; i < Statements.size(); i++)
      Statements[i]->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  DeclVec Declarations;
  StmtVec Statements;
};

class ExpressionStatement : public Statement {
public:
  std::unique_ptr<Expression> &GetExpression() { return Expr; }
  void SetExpression(std::unique_ptr<Expression> e) { Expr = std::move(e); }
  void ASTDump(unsigned tab = 0) override {
    PrintLn("ExpressionStatement", tab);
    Expr->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> Expr;
};

class IfStatement : public Statement {
public:
  std::unique_ptr<Expression> &GetCondition() { return Condition; }
  void SetCondition(std::unique_ptr<Expression> c) { Condition = std::move(c); }

  std::unique_ptr<Statement> &GetIfBody() { return IfBody; }
  void SetIfBody(std::unique_ptr<Statement> ib) { IfBody = std::move(ib); }

  std::unique_ptr<Statement> &GetElseBody() { return ElseBody; }
  void SetElseBody(std::unique_ptr<Statement> eb) { ElseBody = std::move(eb); }

  void ASTDump(unsigned tab = 0) override {
    PrintLn("IfStatement", tab);
    Condition->ASTDump(tab + 2);
    IfBody->ASTDump(tab + 2);
    ElseBody->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> Condition;
  std::unique_ptr<Statement> IfBody;
  std::unique_ptr<Statement> ElseBody = nullptr;
};

class WhileStatement : public Statement {
public:
  std::unique_ptr<Expression> &GetCondition() { return Condition; }
  void SetCondition(std::unique_ptr<Expression> c) { Condition = std::move(c); }

  std::unique_ptr<Statement> &GetBody() { return Body; }
  void SetBody(std::unique_ptr<Statement> b) { Body = std::move(b); }

  void ASTDump(unsigned tab = 0) override {
    PrintLn("WhileStatement", tab);
    Condition->ASTDump(tab + 2);
    Body->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> Condition;
  std::unique_ptr<Statement> Body;
};

class ForStatement : public Statement {
public:
  std::unique_ptr<Expression> &GetInit() { return Init; }
  void SetInit(std::unique_ptr<Expression> c) { Init = std::move(c); }

  std::unique_ptr<Expression> &GetCondition() { return Condition; }
  void SetCondition(std::unique_ptr<Expression> c) { Condition = std::move(c); }

  std::unique_ptr<Expression> &GetIncrement() { return Increment; }
  void SetIncrement(std::unique_ptr<Expression> c) { Increment = std::move(c); }

  std::unique_ptr<Statement> &GetBody() { return Body; }
  void SetBody(std::unique_ptr<Statement> b) { Body = std::move(b); }

  void ASTDump(unsigned tab = 0) override {
    PrintLn("ForStatement", tab);
    Init->ASTDump(tab + 2);
    Condition->ASTDump(tab + 2);
    Increment->ASTDump(tab + 2);
    Body->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> Init;
  std::unique_ptr<Expression> Condition;
  std::unique_ptr<Expression> Increment;
  std::unique_ptr<Statement> Body;
};

class ReturnStatement : public Statement {
public:
  std::unique_ptr<Expression> &GetCondition() {
    assert(HasValue() && "Must have a value to return it.");
    return ReturnValue.value();
  }
  void SetCondition(std::unique_ptr<Expression> v) {
    ReturnValue = std::move(v);
  }
  bool HasValue() { return ReturnValue.has_value(); }

  ReturnStatement() = default;
  ReturnStatement(std::unique_ptr<Expression> e) : ReturnValue(std::move(e)) {}

  void ASTDump(unsigned tab = 0) override {
    PrintLn("ReturnStatement", tab);
    if (ReturnValue)
      ReturnValue.value()->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::optional<std::unique_ptr<Expression>> ReturnValue;
};

class FunctionParameterDeclaration : public Statement {
public:
  std::string &GetName() { return Name; }
  void SetName(std::string &s) { Name = s; }

  Type GetType() { return Ty; }
  void SetType(Type t) { Ty = t; }

  void ASTDump(unsigned tab = 0) override {
    Print("FunctionParameterDeclaration ", tab);
    auto TypeStr = "'" + Ty.ToString() + "' ";
    Print(TypeStr.c_str());
    auto NameStr = "'" + Name + "'";
    PrintLn(NameStr.c_str());
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Name;
  Type Ty;
};

class FunctionDeclaration : public Statement {
  using ParamVec = std::vector<std::unique_ptr<FunctionParameterDeclaration>>;

public:
  Type GetType() { return T; }
  void SetType(Type ft) { T = ft; }

  std::string &GetName() { return Name; }
  void SetName(std::string &s) { Name = s; }

  ParamVec &GetArguments() { return Arguments; }
  void SetArguments(ParamVec &a) { Arguments = std::move(a); }
  void SetArguments(ParamVec &&a) { Arguments = std::move(a); }

  std::unique_ptr<CompoundStatement> &GetBody() { return Body; }
  void SetBody(std::unique_ptr<CompoundStatement> &cs) { Body = std::move(cs); }

  static Type CreateType(const Type &t, const ParamVec &params) {
    Type ResultType(t);
    ResultType.SetTypeKind(Type::Function);

    for (size_t i = 0; i < params.size(); i++) {
      auto t = params[i]->GetType();
      ResultType.GetArgTypes().push_back(t);
    }
    // if there are no arguments then set it to void
    if (params.size() == 0)
      ResultType.GetArgTypes().push_back(Type::Void);

    return ResultType;
  }

  FunctionDeclaration() = delete;

  FunctionDeclaration(Type FT, std::string Name, ParamVec &Args,
                      std::unique_ptr<CompoundStatement> &Body)
      : T(FT), Name(Name), Arguments(std::move(Args)), Body(std::move(Body)) {}

  void ASTDump(unsigned tab = 0) override {
    Print("FunctionDeclaration ", tab);
    auto TypeStr = "'" + T.ToString() + "' ";
    Print(TypeStr.c_str());
    auto NameStr = "'" + Name + "'";
    PrintLn(NameStr.c_str());
    for (size_t i = 0; i < Arguments.size(); i++)
      Arguments[i]->ASTDump(tab + 2);
    Body->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  Type T;
  std::string Name;
  ParamVec Arguments;
  std::unique_ptr<CompoundStatement> Body;
};

class BinaryExpression : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  enum BinaryOperation {
    ASSIGN,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    AND,
    Not,
    EQ,
    LT,
    GT,
    NE,
    ANDL
  };

  BinaryOperation GetOperationKind() {
    switch (Operation.GetKind()) {
    case Token::Equal:
      return ASSIGN;
    case Token::Plus:
      return ADD;
    case Token::Minus:
      return SUB;
    case Token::Astrix:
      return MUL;
    case Token::ForwardSlash:
      return DIV;
    case Token::Percent:
      return MOD;
    case Token::And:
      return AND;
    case Token::Bang:
      return Not;
    case Token::DoubleEqual:
      return EQ;
    case Token::LessThan:
      return LT;
    case Token::GreaterThan:
      return GT;
    case Token::BangEqual:
      return NE;
    case Token::DoubleAnd:
      return ANDL;
    default:
      assert(false && "Invalid binary operator kind.");
      break;
    }
  }

  Token GetOperation() { return Operation; }
  void SetOperation(Token bo) { Operation = bo; }

  ExprPtr &GetLeftExpr() { return Left; }
  void SetLeftExpr(ExprPtr &e) { Left = std::move(e); }

  ExprPtr &GetRightExpr() { return Right; }
  void SetRightExpr(ExprPtr &e) { Right = std::move(e); }

  bool IsConditional() { return GetOperationKind() >= Not; }

  BinaryExpression(ExprPtr L, Token Op, ExprPtr R) {
    Left = std::move(L);
    Operation = Op;
    Right = std::move(R);
    if (IsConditional())
      ResultType = Type(Type::Int);
    else
      ResultType =
          Type(Type::GetStrongestType(Left->GetResultType().GetTypeVariant(),
                                      Right->GetResultType().GetTypeVariant()));
  }

  BinaryExpression() = default;

  void ASTDump(unsigned tab = 0) override {
    Print("BinaryExpression ", tab);
    auto Str = "'" + ResultType.ToString() + "' ";
    Str += "'" + Operation.GetString() + "'";
    PrintLn(Str.c_str());
    Left->ASTDump(tab + 2);
    Right->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  Token Operation;
  std::unique_ptr<Expression> Left;
  std::unique_ptr<Expression> Right;
};

class StructMemberReference : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  std::string GetMemberId() { return MemberIdentifier; }
  void SetMemberId(std::string id) { MemberIdentifier = id; }

  ExprPtr &GetExpr() { return StructTypedExpression; }
  void SetExpr(ExprPtr &e) { StructTypedExpression = std::move(e); }

  StructMemberReference(ExprPtr Expr, std::string Id, size_t Idx) :
    StructTypedExpression(std::move(Expr)), MemberIdentifier(Id), MemberIndex(Idx) {
    auto STEType = StructTypedExpression->GetResultType();
    assert(MemberIndex < STEType.GetTypeList().size());
    this->ResultType = STEType.GetTypeList()[MemberIndex];
  }

  StructMemberReference() {}

  void ASTDump(unsigned tab = 0) override {
    Print("StructMemberReference ", tab);
    auto Str = "'" + ResultType.ToString() + "' ";
    Str += "'." + MemberIdentifier + "'";
    PrintLn(Str.c_str());
    StructTypedExpression->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  ExprPtr StructTypedExpression;
  std::string MemberIdentifier;
  size_t MemberIndex;
};

class UnaryExpression : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  enum UnaryOperation {
    DEREF,
  };

  UnaryOperation GetOperationKind() {
    switch (Operation.GetKind()) {
    case Token::Astrix:
      return DEREF;
    default:
      assert(!"Invalid unary operator kind.");
      break;
    }
  }

  Token GetOperation() { return Operation; }
  void SetOperation(Token bo) { Operation = bo; }

  ExprPtr &GetExpr() { return Expr; }
  void SetExpr(ExprPtr &e) { Expr = std::move(e); }

  UnaryExpression(Token Op, ExprPtr E) {
    Operation = Op;
    Expr = std::move(E);

    if (GetOperationKind() == DEREF) {
      ResultType = Expr->GetResultType();
      ResultType.DecrementPointerLevel();
    } else
      assert(!"Unimplemented!");
  }

  UnaryExpression() = default;

  void ASTDump(unsigned tab = 0) override {
    Print("UnaryExpression ", tab);
    auto Str = "'" + ResultType.ToString() + "' ";
    Str += "'" + Operation.GetString() + "'";
    PrintLn(Str.c_str());
    Expr->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  Token Operation;
  std::unique_ptr<Expression> Expr;
};

class CallExpression : public Expression {
  using ExprVec = std::vector<std::unique_ptr<Expression>>;

public:
  std::string &GetName() { return Name; }
  void SetName(std::string &n) { Name = n; }

  ExprVec &GetArguments() { return Arguments; }
  void SetArguments(ExprVec &a) { Arguments = std::move(a); }

  CallExpression(const std::string &Name, ExprVec &Args, Type T)
      : Name(Name), Arguments(std::move(Args)), Expression(std::move(T)) {}

  void ASTDump(unsigned tab = 0) override {
    Print("CallExpression ", tab);
    auto Str = "'" + ResultType.ToString() + "' ";
    Str += "'" + Name + "'";
    PrintLn(Str.c_str());
    for (size_t i = 0; i < Arguments.size(); i++)
      Arguments[i]->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Name;
  ExprVec Arguments;
};

class ReferenceExpression : public Expression {
public:
  std::string &GetIdentifier() { return Identifier; }
  void SetIdentifier(std::string &id) { Identifier = id; }

  ReferenceExpression(Token t) { Identifier = t.GetString(); }


  void ASTDump(unsigned tab = 0) override {
    Print("ReferenceExpression ", tab);
    auto Str = "'" + ResultType.ToString() + "' ";
    Str += "'" + Identifier + "'";
    PrintLn(Str.c_str());
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Identifier;
};

class IntegerLiteralExpression : public Expression {
public:
  unsigned GetValue() { return IntValue; }
  void SetValue(uint64_t v) { IntValue = v; }

  IntegerLiteralExpression(uint64_t v) : IntValue(v) {
    SetType(Type(Type::Int));
  }
  IntegerLiteralExpression() = delete;

  void ASTDump(unsigned tab = 0) override {
    Print("IntegerLiteralExpression ", tab);
    auto TyStr = "'" + ResultType.ToString() + "' ";
    Print(TyStr.c_str());
    auto ValStr = "'" + std::to_string(IntValue) + "'";
    PrintLn(ValStr.c_str());
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  uint64_t IntValue;
};

class FloatLiteralExpression : public Expression {
public:
  double GetValue() { return FPValue; }
  void SetValue(double v) { FPValue = v; }

  FloatLiteralExpression(double v) : FPValue(v) { SetType(Type(Type::Double)); }
  FloatLiteralExpression() = delete;

  void ASTDump(unsigned tab = 0) override {
    Print("FloatLiteralExpression ", tab);
    auto TyStr = "'" + ResultType.ToString() + "' ";
    Print(TyStr.c_str());
    auto ValStr = "'" + std::to_string(FPValue) + "'";
    PrintLn(ValStr.c_str());
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  double FPValue;
};

class ArrayExpression : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  ExprPtr &GetIndexExpression() { return IndexExpression; }
  void SetIndexExpression(ExprPtr &e) { IndexExpression = std::move(e); }

  ArrayExpression(ExprPtr &Base, ExprPtr &Index, Type Ct = Type())
      : BaseExpression(std::move(Base)), IndexExpression(std::move(Index)) {
    ResultType = Ct;
  }

  void SetLValueness(bool p) { IsLValue = p; }
  bool GetLValueness() { return IsLValue; }

  void ASTDump(unsigned tab = 0) override {
    Print("ArrayExpression ", tab);
    auto Str = "'" + ResultType.ToString() + "' ";
    //Str += "'" + Identifier.GetString() + "'";
    PrintLn(Str.c_str());
    IndexExpression->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  bool IsLValue = false;
  ExprPtr BaseExpression;
  ExprPtr IndexExpression;
};

class ImplicitCastExpression : public Expression {
public:
  ImplicitCastExpression(std::unique_ptr<Expression> e, Type t)
      : CastableExpression(std::move(e)), Expression(t.GetTypeVariant()) {}

  Type GetSourceType() { return CastableExpression->GetResultType(); }
  std::unique_ptr<Expression> &GetCastableExpression() {
    return CastableExpression;
  }

  void ASTDump(unsigned tab = 0) override {
    Print("ImplicitCastExpression ", tab);
    auto Str = "'" + ResultType.ToString() + "'";
    PrintLn(Str.c_str());
    CastableExpression->ASTDump(tab + 2);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> CastableExpression;
};

class TranslationUnit : public Statement {
public:
  std::vector<std::unique_ptr<Statement>> &GetDeclarations() {
    return Declarations;
  }
  void SetDeclarations(std::vector<std::unique_ptr<Statement>> s) {
    Declarations = std::move(s);
  }
  void AddDeclaration(std::unique_ptr<Statement> s) {
    Declarations.push_back(std::move(s));
  }

  TranslationUnit() = default;

  TranslationUnit(std::vector<std::unique_ptr<Statement>> s)
      : Declarations(std::move(s)) {}

  void ASTDump(unsigned tab = 0) override {
    PrintLn("TranslationUnit", tab);
    for (size_t i = 0; i < Declarations.size(); i++)
      Declarations[i]->ASTDump(tab + 2);
    PrintLn("");
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::vector<std::unique_ptr<Statement>> Declarations;
};

#endif