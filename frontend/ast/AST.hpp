#ifndef AST_HPP
#define AST_HPP

#include "../lexer/Token.hpp"
#include "Type.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

static void PrintImpl(const char *str, unsigned tab = 0, bool newline = false) {
  for (int i = 0; i < tab; i++)
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
  virtual void ASTDump(unsigned tab = 0) { PrintLn("Node"); }
};

class Statement : public Node {
public:
  void ASTDump(unsigned tab = 0) override { PrintLn("Statement", tab); }
};

class Expression : public Node {
public:
  Expression() = default;
  Expression(ComplexType t) : ResultType(std::move(t)) {}
  Expression(Type::VariantKind vk) : ResultType(vk) {}
  ComplexType GetResultType() { return ResultType; }
  void SetType(ComplexType t) { ResultType = t; }
  void ASTDump(unsigned tab = 0) override { PrintLn("Expression", tab); }

protected:
  ComplexType ResultType;
};

class VariableDeclaration : public Statement {
public:
  std::string &GetName() { return Name; }
  void SetName(std::string &s) { Name = s; }

  ArrayType GetType() { return Ty; }
  void SetType(ArrayType t) { Ty = t; }

  VariableDeclaration(std::string &Name, Type Ty, std::vector<unsigned> Dim)
      : Name(Name), Ty(Ty, std::move(Dim)) {}

  VariableDeclaration(std::string &Name, ArrayType Ty) : Name(Name), Ty(Ty) {}

  VariableDeclaration() = default;

  void ASTDump(unsigned tab = 0) override {
    Print("VariableDeclaration ", tab);
    auto TypeStr = "'" + Ty.ToString() + "' ";
    Print(TypeStr.c_str());
    auto NameStr = "'" + Name + "'";
    PrintLn(NameStr.c_str());
  }

private:
  std::string Name;
  ArrayType Ty;
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
    for (int i = 0; i < Declarations.size(); i++)
      Declarations[i]->ASTDump(tab + 2);
    for (int i = 0; i < Statements.size(); i++)
      Statements[i]->ASTDump(tab + 2);
  }

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

private:
  std::unique_ptr<Expression> Condition;
  std::unique_ptr<Statement> IfBody;
  std::unique_ptr<Statement> ElseBody;
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

private:
  std::unique_ptr<Expression> Condition;
  std::unique_ptr<Statement> Body;
};

class ReturnStatement : public Statement {
public:
  std::unique_ptr<Expression> &GetCondition() {
    assert(HasValue() && "Must have a value to return it.");
    return Value.value();
  }
  void SetCondition(std::unique_ptr<Expression> v) { Value = std::move(v); }
  bool HasValue() { return Value.has_value(); }

  ReturnStatement() = default;
  ReturnStatement(std::unique_ptr<Expression> e) : Value(std::move(e)) {}

  void ASTDump(unsigned tab = 0) override {
    PrintLn("ReturnStatement", tab);
    if (Value)
      Value.value()->ASTDump(tab + 2);
  }

private:
  std::optional<std::unique_ptr<Expression>> Value;
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

private:
  std::string Name;
  Type Ty;
};

class FunctionDeclaration : public Statement {
  using ParamVec = std::vector<std::unique_ptr<FunctionParameterDeclaration>>;

public:
  FunctionType GetType() { return Type; }
  void SetType(FunctionType ft) { Type = ft; }

  std::string &GetName() { return Name; }
  void SetName(std::string &s) { Name = s; }

  ParamVec &GetArguments() { return Arguments; }
  void SetArguments(ParamVec &a) { Arguments = std::move(a); }
  void SetArguments(ParamVec &&a) { Arguments = std::move(a); }

  std::unique_ptr<CompoundStatement> &GetBody() { return Body; }
  void SetBody(std::unique_ptr<CompoundStatement> &cs) { Body = std::move(cs); }

  void CalcArgumentTypes() {
    for (int i = 0; i < Arguments.size(); i++) {
      auto t = Arguments[i]->GetType().GetTypeVariant();
      Type.GetArgumentTypes().push_back(t);
    }
    // if there are no arguments then set it to void
    if (Arguments.size() == 0)
      Type.GetArgumentTypes().push_back(Type::Void);
  }

  FunctionDeclaration() = delete;

  FunctionDeclaration(FunctionType FT, std::string Name, ParamVec &Args,
                      std::unique_ptr<CompoundStatement> &Body)
      : Type(FT), Name(Name), Arguments(std::move(Args)),
        Body(std::move(Body)) {
    CalcArgumentTypes();
  }

  void ASTDump(unsigned tab = 0) override {
    Print("FunctionDeclaration ", tab);
    auto TypeStr = "'" + Type.ToString() + "' ";
    Print(TypeStr.c_str());
    auto NameStr = "'" + Name + "'";
    PrintLn(NameStr.c_str());
    for (int i = 0; i < Arguments.size(); i++)
      Arguments[i]->ASTDump(tab + 2);
    Body->ASTDump(tab + 2);
  }

private:
  FunctionType Type;
  std::string Name;
  ParamVec Arguments;
  std::unique_ptr<CompoundStatement> Body;
};

class BinaryExpression : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  enum BinaryOperation {
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Equal,
    Less,
    Greater,
    NotEqual,
    LogicalAnd
  };

  BinaryOperation GetOperationKind() {
    switch (Operation.GetKind()) {
    case Token::Equal:
      return Assign;
    case Token::Plus:
      return Add;
    case Token::Minus:
      return Sub;
    case Token::Astrix:
      return Mul;
    case Token::ForwardSlash:
      return Div;
    case Token::Percent:
      return Mod;
    case Token::DoubleEqual:
      return Equal;
    case Token::LessThan:
      return Less;
    case Token::GreaterThan:
      return Greater;
    case Token::BangEqual:
      return NotEqual;
    case Token::DoubleAnd:
      return LogicalAnd;
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

  bool IsConditional() { return GetOperationKind() >= Equal; }

  BinaryExpression(ExprPtr L, Token Op, ExprPtr R) {
    Left = std::move(L);
    Operation = Op;
    Right = std::move(R);
    ResultType = ComplexType(
        Type::GetStrongestType(Left->GetResultType().GetTypeVariant(),
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

private:
  Token Operation;
  std::unique_ptr<Expression> Left;
  std::unique_ptr<Expression> Right;
};

class CallExpression : public Expression {
  using ExprVec = std::vector<std::unique_ptr<Expression>>;

public:
  std::string &GetName() { return Name; }
  void SetName(std::string &n) { Name = n; }

  ExprVec &GetArguments() { return Arguments; }
  void SetArguments(ExprVec &a) { Arguments = std::move(a); }

  CallExpression(const std::string &Name, ExprVec &Args, ComplexType T)
      : Name(Name), Arguments(std::move(Args)), Expression(std::move(T)) {}

  void ASTDump(unsigned tab = 0) override {
    Print("CallExpression ", tab);
    auto Str = "'" + ResultType.ToString() + "' ";
    Str += "'" + Name + "'";
    PrintLn(Str.c_str());
    for (int i = 0; i < Arguments.size(); i++)
      Arguments[i]->ASTDump(tab + 2);
  }

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

private:
  std::string Identifier;
};

class IntegerLiteralExpression : public Expression {
public:
  unsigned GetValue() { return Value; }
  void SetValue(unsigned v) { Value = v; }

  IntegerLiteralExpression(unsigned v) : Value(v) {
    SetType(ComplexType(Type::Int));
  }
  IntegerLiteralExpression() = delete;

  void ASTDump(unsigned tab = 0) override {
    Print("IntegerLiteralExpression ", tab);
    auto TyStr = "'" + ResultType.ToString() + "' ";
    Print(TyStr.c_str());
    auto ValStr = "'" + std::to_string(Value) + "'";
    PrintLn(ValStr.c_str());
  }

private:
  unsigned Value;
};

class FloatLiteralExpression : public Expression {
public:
  double GetValue() { return Value; }
  void SetValue(double v) { Value = v; }

  FloatLiteralExpression(double v) : Value(v) {
    SetType(ComplexType(Type::Double));
  }
  FloatLiteralExpression() = delete;

  void ASTDump(unsigned tab = 0) override {
    Print("FloatLiteralExpression ", tab);
    auto TyStr = "'" + ResultType.ToString() + "' ";
    Print(TyStr.c_str());
    auto ValStr = "'" + std::to_string(Value) + "'";
    PrintLn(ValStr.c_str());
  }

private:
  double Value;
};

class ArrayExpression : public Expression {
  using ExprVec = std::vector<std::unique_ptr<Expression>>;

public:
  Token &GetIdentifier() { return Identifier; }
  void SetIdentifier(Token &id) { Identifier = id; }

  ExprVec &GetIndexExpression() { return IndexExpressions; }
  void SetIndexExpression(ExprVec &e) { IndexExpressions = std::move(e); }

  ArrayExpression(const Token &Id, ExprVec &IEs, ComplexType Ct = ComplexType())
      : Identifier(Id), IndexExpressions(std::move(IEs)) {
    ResultType = Ct;
  }

  void ASTDump(unsigned tab = 0) override {
    Print("ArrayExpression ", tab);
    auto Str = "'" + ResultType.ToString() + "' ";
    Str += "'" + Identifier.GetString() + "'";
    PrintLn(Str.c_str());
    for (int i = 0; i < IndexExpressions.size(); i++)
      IndexExpressions[i]->ASTDump(tab + 2);
  }

private:
  Token Identifier;
  ExprVec IndexExpressions;
};

class ImplicitCastExpression : public Expression {
public:
  ImplicitCastExpression(std::unique_ptr<Expression> e, Type::VariantKind rt)
      : CastableExpression(std::move(e)), Expression(rt) {}

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
    for (int i = 0; i < Declarations.size(); i++)
      Declarations[i]->ASTDump(tab + 2);
  }

private:
  std::vector<std::unique_ptr<Statement>> Declarations;
};

#endif