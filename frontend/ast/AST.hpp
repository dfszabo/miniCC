#ifndef AST_HPP
#define AST_HPP

#include "../../middle_end/IR/IRFactory.hpp"
#include "../../middle_end/IR/Value.hpp"
#include "../lexer/Token.hpp"
#include "ASTVisitor.hpp"
#include "Type.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Node {
public:
  virtual ~Node(){};
  virtual void Accept(ASTVisitor *visitor) const { return; }

  virtual Value *IRCodegen(IRFactory *IRF) {
    assert(!"Must be a child node type");
    return nullptr;
  }
};

class Statement : public Node {
public:
  enum StmtInfo {
    NONE = 0,
    RETURN = 1,
  };

  void AddInfo(unsigned Bit) { InfoBits |= Bit; }

  bool IsRet() { return !!(InfoBits & RETURN); }

private:
  unsigned InfoBits = 0;
};

class Expression : public Node {
public:
  Expression() = default;
  Expression(Type t) : ResultType(std::move(t)) {}
  Expression(Type::VariantKind vk) : ResultType(vk) {}
  Type &GetResultType() { return ResultType; }
  Type const &GetResultType() const { return ResultType; }
  void SetType(Type t) { ResultType = t; }

  void SetLValueness(bool p) { IsLValue = p; }
  bool GetLValueness() { return IsLValue; }

protected:
  bool IsLValue = false;
  Type ResultType;
};

class VariableDeclaration : public Statement {
public:
  std::string const &GetName() const { return Name; }
  void SetName(std::string &s) { Name = s; }

  Type GetType() const { return AType; }
  void SetType(Type t) { AType = t; }

  std::unique_ptr<Expression> &GetInitExpr() { return Init; }
  std::unique_ptr<Expression> const &GetInitExpr() const { return Init; }
  void SetInitExpr(std::unique_ptr<Expression> e) { Init = std::move(e); }

  VariableDeclaration(std::string &Name, Type Ty, std::vector<unsigned> Dim)
      : Name(Name), AType(Ty, std::move(Dim)) {}

  VariableDeclaration(std::string &Name, Type Ty) : Name(Name), AType(Ty) {}
  VariableDeclaration(std::string &Name, Type Ty, std::unique_ptr<Expression> E)
      : Name(Name), AType(Ty), Init(std::move(E)) {}

  VariableDeclaration() = default;

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitVariableDeclaration(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Name;
  Type AType;
  std::unique_ptr<Expression> Init = nullptr;
};

class MemberDeclaration : public Statement {
public:
  std::string const &GetName() const { return Name; }
  void SetName(std::string &s) { Name = s; }

  Type GetType() const { return AType; }
  void SetType(Type t) { AType = t; }

  MemberDeclaration(std::string &Name, Type Ty, std::vector<unsigned> Dim)
      : Name(Name), AType(Ty, std::move(Dim)) {}

  MemberDeclaration(std::string &Name, Type Ty) : Name(Name), AType(Ty) {}

  MemberDeclaration() = default;

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitMemberDeclaration(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Name;
  Type AType;
};

class StructDeclaration : public Statement {
public:
  std::string const &GetName() const { return Name; }
  void SetName(std::string &s) { Name = s; }

  std::vector<std::unique_ptr<MemberDeclaration>> const &GetMembers() const {
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

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitStructDeclaration(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  Type SType;
  std::string Name;
  std::vector<std::unique_ptr<MemberDeclaration>> Members;
};

class EnumDeclaration : public Statement {
public:
  using EnumList = std::vector<std::pair<std::string, int>>;

  EnumDeclaration(Type &BaseType, EnumList Enumerators)
      : BaseType(BaseType), Enumerators(std::move(Enumerators)) {}

  EnumDeclaration(EnumList Enumerators) : Enumerators(std::move(Enumerators)) {}

  Type GetBaseType() const { return BaseType; }

  EnumList const &GetEnumerators() const { return Enumerators; }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitEnumDeclaration(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  Type BaseType = Type(Type::Int);
  EnumList Enumerators;
};

class CompoundStatement : public Statement {
  using StmtVec = std::vector<std::unique_ptr<Statement>>;

public:
  StmtVec &GetStatements() { return Statements; }
  StmtVec const &GetStatements() const { return Statements; }
  void SetStatements(StmtVec &s) { Statements = std::move(s); }
  void AddStatement(std::unique_ptr<Statement> &s) {
    Statements.push_back(std::move(s));
  }

  CompoundStatement(StmtVec &Stats) : Statements(std::move(Stats)) {}

  CompoundStatement() = delete;

  CompoundStatement(const CompoundStatement &) = delete;
  CompoundStatement &operator=(const CompoundStatement &) = delete;

  CompoundStatement(CompoundStatement &&) = default;

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitCompoundStatement(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  StmtVec Statements;
};

class ExpressionStatement : public Statement {
public:
  std::unique_ptr<Expression> const &GetExpression() const { return Expr; }
  void SetExpression(std::unique_ptr<Expression> e) { Expr = std::move(e); }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitExpressionStatement(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> Expr;
};

class IfStatement : public Statement {
public:
  std::unique_ptr<Expression> const &GetCondition() const { return Condition; }
  void SetCondition(std::unique_ptr<Expression> c) { Condition = std::move(c); }

  std::unique_ptr<Statement> const &GetIfBody() const { return IfBody; }
  void SetIfBody(std::unique_ptr<Statement> ib) { IfBody = std::move(ib); }

  std::unique_ptr<Statement> const &GetElseBody() const { return ElseBody; }
  void SetElseBody(std::unique_ptr<Statement> eb) { ElseBody = std::move(eb); }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitIfStatement(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> Condition;
  std::unique_ptr<Statement> IfBody;
  std::unique_ptr<Statement> ElseBody = nullptr;
};

class SwitchStatement : public Statement {
public:
  using VecOfStmts = std::vector<std::unique_ptr<Statement>>;
  using VecOfCasesData = std::vector<std::pair<int, VecOfStmts>>;

  std::unique_ptr<Expression> const &GetCondition() const { return Condition; }
  void SetCondition(std::unique_ptr<Expression> c) { Condition = std::move(c); }

  VecOfCasesData const &GetCaseBodies() const { return Cases; }
  void SetCaseBodies(VecOfCasesData c) { Cases = std::move(c); }

  VecOfStmts const &GetDefaultBody() const { return DefaultBody; }
  void SetDefaultBody(VecOfStmts db) { DefaultBody = std::move(db); }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitSwitchStatement(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> Condition;
  VecOfCasesData Cases;
  VecOfStmts DefaultBody;
};

class WhileStatement : public Statement {
public:
  std::unique_ptr<Expression> const &GetCondition() const { return Condition; }
  void SetCondition(std::unique_ptr<Expression> c) { Condition = std::move(c); }

  std::unique_ptr<Statement> const &GetBody() const { return Body; }
  void SetBody(std::unique_ptr<Statement> b) { Body = std::move(b); }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitWhileStatement(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> Condition;
  std::unique_ptr<Statement> Body;
};

class ForStatement : public Statement {
public:
  std::unique_ptr<Statement> const &GetVarDecl() const { return VarDecl; }
  void SetVarDecl(std::unique_ptr<Statement> v) { VarDecl = std::move(v); }

  std::unique_ptr<Expression> const &GetInit() const { return Init; }
  void SetInit(std::unique_ptr<Expression> c) { Init = std::move(c); }

  std::unique_ptr<Expression> const &GetCondition() const { return Condition; }
  void SetCondition(std::unique_ptr<Expression> c) { Condition = std::move(c); }

  std::unique_ptr<Expression> const &GetIncrement() const { return Increment; }
  void SetIncrement(std::unique_ptr<Expression> c) { Increment = std::move(c); }

  std::unique_ptr<Statement> const &GetBody() const { return Body; }
  void SetBody(std::unique_ptr<Statement> b) { Body = std::move(b); }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitForStatement(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Statement> VarDecl = nullptr;
  std::unique_ptr<Expression> Init = nullptr;
  std::unique_ptr<Expression> Condition;
  std::unique_ptr<Expression> Increment;
  std::unique_ptr<Statement> Body;
};

class ReturnStatement : public Statement {
public:
  std::unique_ptr<Expression> &GetRetVal() {
    assert(HasValue() && "Must have a value to return it.");
    return ReturnValue.value();
  }

  std::unique_ptr<Expression> const &GetRetVal() const {
    assert(HasValue() && "Must have a value to return it.");
    return ReturnValue.value();
  }

  void SetRetVal(std::unique_ptr<Expression> v) { ReturnValue = std::move(v); }
  bool HasValue() const { return ReturnValue.has_value(); }

  ReturnStatement() { AddInfo(Statement::RETURN); }
  ReturnStatement(std::unique_ptr<Expression> e) : ReturnValue(std::move(e)) {
    AddInfo(Statement::RETURN);
  }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitReturnStatement(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::optional<std::unique_ptr<Expression>> ReturnValue;
};

class BreakStatement : public Statement {
public:
  BreakStatement() = default;

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitBreakStatement(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;
};

class ContinueStatement : public Statement {
public:
  ContinueStatement() = default;

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitContinueStatement(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;
};

class FunctionParameterDeclaration : public Statement {
public:
  std::string const &GetName() const { return Name; }
  void SetName(std::string &s) { Name = s; }

  Type GetType() const { return Ty; }
  void SetType(Type t) { Ty = t; }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitFunctionParameterDeclaration(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Name;
  Type Ty;
};

class FunctionDeclaration : public Statement {
  using ParamVec = std::vector<std::unique_ptr<FunctionParameterDeclaration>>;

public:
  Type GetType() const { return T; }
  void SetType(Type ft) { T = ft; }

  std::string const &GetName() const { return Name; }
  void SetName(std::string &s) { Name = s; }

  ParamVec const &GetArguments() const { return Arguments; }
  void SetArguments(ParamVec &a) { Arguments = std::move(a); }
  void SetArguments(ParamVec &&a) { Arguments = std::move(a); }

  std::unique_ptr<CompoundStatement> const &GetBody() const { return Body; }
  void SetBody(std::unique_ptr<CompoundStatement> &cs) { Body = std::move(cs); }

  static Type CreateType(const Type &t, const ParamVec &params) {
    Type ResultType(t);

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
                      std::unique_ptr<CompoundStatement> &Body, unsigned RetNum)
      : T(FT), Name(Name), Arguments(std::move(Args)), Body(std::move(Body)),
        ReturnsNumber(RetNum) {}

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitFunctionDeclaration(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  Type T;
  std::string Name;
  ParamVec Arguments;
  std::unique_ptr<CompoundStatement> Body;
  unsigned ReturnsNumber;
};

class BinaryExpression : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  enum BinaryOperation {
    ASSIGN,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN,
    AND_ASSIGN,
    OR_ASSIGN,
    XOR_ASSIGN,
    LSL_ASSIGN,
    LSR_ASSIGN,
    LSL,
    LSR,
    ADD,
    SUB,
    MUL,
    DIV,
    DIVU,
    MOD,
    MODU,
    AND,
    OR,
    XOR,
    Not,
    EQ,
    LT,
    GT,
    NE,
    GE,
    LE,
    ANDL
  };

  BinaryOperation GetOperationKind() {
    switch (Operation.GetKind()) {
    case Token::Equal:
      return ASSIGN;
    case Token::PlusEqual:
      return ADD_ASSIGN;
    case Token::MinusEqual:
      return SUB_ASSIGN;
    case Token::AstrixEqual:
      return MUL_ASSIGN;
    case Token::ForwardSlashEqual:
      return DIV_ASSIGN;
    case Token::PercentEqual:
      return MOD_ASSIGN;
    case Token::AndEqual:
      return AND_ASSIGN;
    case Token::OrEqual:
      return OR_ASSIGN;
    case Token::CaretEqual:
      return XOR_ASSIGN;
    case Token::LessThanLessThanEqual:
      return LSL_ASSIGN;
    case Token::GreaterThanGreaterThanEqual:
      return LSR_ASSIGN;
    case Token::LessThanLessThan:
      return LSL;
    case Token::GreaterThanGreaterThan:
      return LSR;
    case Token::Plus:
      return ADD;
    case Token::Minus:
      return SUB;
    case Token::Astrix:
      return MUL;
    case Token::ForwardSlash:
      if (GetResultType().IsUnsigned())
        return DIVU;
      return DIV;
    case Token::Percent:
      if (GetResultType().IsUnsigned())
        return MODU;
      return MOD;
    case Token::And:
      return AND;
    case Token::Or:
      return OR;
    case Token::Caret:
      return XOR;
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
    case Token::GreaterEqual:
      return GE;
    case Token::LessEqual:
      return LE;
    case Token::DoubleAnd:
      return ANDL;
    default:
      assert(false && "Invalid binary operator kind.");
      break;
    }
  }

  Token GetOperation() const { return Operation; }
  void SetOperation(Token bo) { Operation = bo; }

  ExprPtr const &GetLeftExpr() const { return Left; }
  void SetLeftExpr(ExprPtr &e) { Left = std::move(e); }

  ExprPtr const &GetRightExpr() const { return Right; }
  void SetRightExpr(ExprPtr &e) { Right = std::move(e); }

  bool IsConditional() { return GetOperationKind() >= Not; }

  BinaryExpression(ExprPtr L, Token Op, ExprPtr R) {
    Left = std::move(L);
    Operation = Op;
    Right = std::move(R);
    if (IsConditional())
      ResultType = Type(Type::Int);
    else {
      auto Strongest =
          Type::GetStrongestType(Left->GetResultType().GetTypeVariant(),
                                 Right->GetResultType().GetTypeVariant());
      ResultType =
          Type(Type::GetStrongestType(Strongest.GetTypeVariant(), Type::Int));
    }
  }

  BinaryExpression() = default;

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitBinaryExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  Token Operation;
  std::unique_ptr<Expression> Left;
  std::unique_ptr<Expression> Right;
};

class TernaryExpression : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  ExprPtr const &GetCondition() const { return Condition; }
  void SetCondition(ExprPtr &e) { Condition = std::move(e); }

  ExprPtr const &GetExprIfTrue() const { return ExprIfTrue; }
  void SetExprIfTrue(ExprPtr &e) { ExprIfTrue = std::move(e); }

  ExprPtr const &GetExprIfFalse() const { return ExprIfFalse; }
  void SetExprIfFalse(ExprPtr &e) { ExprIfFalse = std::move(e); }

  TernaryExpression() = default;

  TernaryExpression(ExprPtr &Cond, ExprPtr &True, ExprPtr &False)
      : Condition(std::move(Cond)), ExprIfTrue(std::move(True)),
        ExprIfFalse(std::move(False)) {
    ResultType = ExprIfTrue->GetResultType();
  }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitTernaryExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  ExprPtr Condition;
  ExprPtr ExprIfTrue;
  ExprPtr ExprIfFalse;
};

class StructMemberReference : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  std::string GetMemberId() const { return MemberIdentifier; }
  void SetMemberId(std::string id) { MemberIdentifier = id; }

  ExprPtr const &GetExpr() const { return StructTypedExpression; }
  void SetExpr(ExprPtr &e) { StructTypedExpression = std::move(e); }

  StructMemberReference(ExprPtr Expr, std::string Id, size_t Idx)
      : StructTypedExpression(std::move(Expr)), MemberIdentifier(Id),
        MemberIndex(Idx) {
    auto STEType = StructTypedExpression->GetResultType();
    assert(MemberIndex < STEType.GetTypeList().size());
    this->ResultType = STEType.GetTypeList()[MemberIndex];
  }

  StructMemberReference() {}

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitStructMemberReference(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  ExprPtr StructTypedExpression;
  std::string MemberIdentifier;
  size_t MemberIndex;
};

class StructInitExpression : public Expression {
public:
  using UintList = std::vector<unsigned>;
  using ExprPtrList = std::vector<std::unique_ptr<Expression>>;

  UintList &GetMemberOrdering() { return MemberOrdering; }
  void SetMemberOrdering(UintList l) { MemberOrdering = l; }

  ExprPtrList const &GetInitList() const { return InitValues; }
  void SetInitList(ExprPtrList &e) { InitValues = std::move(e); }

  StructInitExpression(Type ResultType, ExprPtrList InitList,
                       UintList InitOrder)
      : InitValues(std::move(InitList)), MemberOrdering(std::move(InitOrder)) {
    this->ResultType = ResultType;
  }

  StructInitExpression() {}

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitStructInitExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  UintList MemberOrdering;
  ExprPtrList InitValues;
};

class UnaryExpression : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  enum UnaryOperation {
    ADDRESS,
    DEREF,
    MINUS,
    NOT,
    POST_INCREMENT,
    POST_DECREMENT,
    PRE_INCREMENT,
    PRE_DECREMENT,
    SIZEOF,
  };

  UnaryOperation GetOperationKind() const {
    switch (Operation.GetKind()) {
    case Token::And:
      return ADDRESS;
    case Token::Astrix:
      return DEREF;
    case Token::Minus:
      return MINUS;
    case Token::Bang:
      return NOT;
    case Token::PlusPlus:
      return IsPostFix ? POST_INCREMENT : PRE_INCREMENT;
    case Token::MinusMinus:
      return IsPostFix ? POST_DECREMENT : PRE_DECREMENT;
    case Token::Sizeof:
      return SIZEOF;
    default:
      assert(!"Invalid unary operator kind.");
      break;
    }
  }

  Token GetOperation() const { return Operation; }
  void SetOperation(Token bo) { Operation = bo; }

  ExprPtr const &GetExpr() const { return Expr; }
  void SetExpr(ExprPtr &e) { Expr = std::move(e); }

  UnaryExpression(Token Op, ExprPtr E, bool PostFix = false) {
    Operation = Op;
    if (E)
      Expr = std::move(E);
    IsPostFix = PostFix;

    switch (GetOperationKind()) {
    case ADDRESS:
      ResultType = Expr->GetResultType();
      ResultType.IncrementPointerLevel();
      break;
    case DEREF:
      ResultType = Expr->GetResultType();
      ResultType.DecrementPointerLevel();
      break;
    case NOT:
      ResultType = Type(Type::Int);
      break;
    case MINUS:
    case POST_DECREMENT:
    case POST_INCREMENT:
    case PRE_DECREMENT:
    case PRE_INCREMENT:
      ResultType = Expr->GetResultType();
      break;
    case SIZEOF:
      if (Expr)
        ResultType = Expr->GetResultType();
      break;
    default:
      assert(!"Unimplemented!");
    }
  }

  UnaryExpression() = default;

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitUnaryExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  Token Operation;
  std::unique_ptr<Expression> Expr = nullptr;
  bool IsPostFix = false;
};

class CallExpression : public Expression {
  using ExprVec = std::vector<std::unique_ptr<Expression>>;

public:
  std::string const &GetName() const { return Name; }
  void SetName(std::string &n) { Name = n; }

  ExprVec const &GetArguments() const { return Arguments; }
  void SetArguments(ExprVec &a) { Arguments = std::move(a); }

  CallExpression(const std::string &Name, ExprVec &Args, Type T)
      : Name(Name), Arguments(std::move(Args)), Expression(std::move(T)) {}

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitCallExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Name;
  ExprVec Arguments;
};

class ReferenceExpression : public Expression {
public:
  std::string &GetIdentifier() { return Identifier; }
  std::string const &GetIdentifier() const { return Identifier; }
  void SetIdentifier(std::string &id) { Identifier = id; }

  ReferenceExpression(Token t) { Identifier = t.GetString(); }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitReferenceExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string Identifier;
};

class IntegerLiteralExpression : public Expression {
public:
  unsigned GetValue() { return IntValue; }
  int64_t GetSIntValue() const { return IntValue; }
  uint64_t GetUIntValue() const { return IntValue; }
  void SetValue(uint64_t v) { IntValue = v; }

  IntegerLiteralExpression(uint64_t v) : IntValue(v) {
    SetType(Type(Type::Int));
  }
  IntegerLiteralExpression() = delete;

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitIntegerLiteralExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  uint64_t IntValue;
};

class FloatLiteralExpression : public Expression {
public:
  double GetValue() const { return FPValue; }
  void SetValue(double v) { FPValue = v; }

  FloatLiteralExpression(double v) : FPValue(v) { SetType(Type(Type::Double)); }
  FloatLiteralExpression() = delete;

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitFloatLiteralExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  double FPValue;
};

class StringLiteralExpression : public Expression {
public:
  std::string GetValue() const { return StringValue; }
  void SetValue(std::string v) { StringValue = v; }

  StringLiteralExpression(std::string s) : StringValue(s) {
    std::vector<unsigned> d = {(unsigned)s.length() + 1};
    // string literal is a char array
    SetType(Type(Type::Char, std::move(d)));
  }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitStringLiteralExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::string StringValue;
};

class ArrayExpression : public Expression {
  using ExprPtr = std::unique_ptr<Expression>;

public:
  ExprPtr const &GetIndexExpression() const { return IndexExpression; }
  void SetIndexExpression(ExprPtr &e) { IndexExpression = std::move(e); }

  ArrayExpression(ExprPtr &Base, ExprPtr &Index, Type Ct = Type())
      : BaseExpression(std::move(Base)), IndexExpression(std::move(Index)) {
    ResultType = Ct;
  }

  void SetLValueness(bool p) { IsLValue = p; }
  bool GetLValueness() { return IsLValue; }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitArrayExpression(this);
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
      : CastableExpression(std::move(e)), Expression(t) {}

  Type GetSourceType() { return CastableExpression->GetResultType(); }
  std::unique_ptr<Expression> &GetCastableExpression() {
    return CastableExpression;
  }

  std::unique_ptr<Expression> const &GetCastableExpression() const {
    return CastableExpression;
  }

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitImplicitCastExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::unique_ptr<Expression> CastableExpression;
};

class InitializerListExpression : public Expression {
  using ExprList = std::vector<std::unique_ptr<Expression>>;

public:
  ExprList &GetExprList() { return Expressions; }
  ExprList const &GetExprList() const { return Expressions; }
  void SetExprList(ExprList &e) { Expressions = std::move(e); }

  InitializerListExpression(ExprList EL) : Expressions(std::move(EL)) {}

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitInitializerListExpression(this);
  }

  Value *IRCodegen(IRFactory *IRF) override { return nullptr; }

private:
  ExprList Expressions;
};

class TranslationUnit : public Statement {
public:
  std::vector<std::unique_ptr<Statement>> const &GetDeclarations() const {
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

  void Accept(ASTVisitor *visitor) const override {
    visitor->VisitTranslationUnit(this);
  }

  Value *IRCodegen(IRFactory *IRF) override;

private:
  std::vector<std::unique_ptr<Statement>> Declarations;
};

#endif