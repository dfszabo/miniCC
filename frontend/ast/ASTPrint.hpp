#ifndef AST_PRINT_HPP
#define AST_PRINT_HPP

#include "ASTVisitor.hpp"

class ASTPrint : public ASTVisitor {
private:
  int tab = 0;

public:
  ASTPrint();
  void VisitVariableDeclaration(const VariableDeclaration *node) override;
  void VisitMemberDeclaration(const MemberDeclaration *node) override;
  void VisitStructDeclaration(const StructDeclaration *node) override;
  void VisitEnumDeclaration(const EnumDeclaration *node) override;
  void VisitCompoundStatement(const CompoundStatement *node) override;
  void VisitExpressionStatement(const ExpressionStatement *node) override;
  void VisitIfStatement(const IfStatement *node) override;
  void VisitSwitchStatement(const SwitchStatement *node) override;
  void VisitWhileStatement(const WhileStatement *node) override;
  void VisitForStatement(const ForStatement *node) override;
  void VisitReturnStatement(const ReturnStatement *node) override;
  void VisitBreakStatement(const BreakStatement *node) override;
  void VisitContinueStatement(const ContinueStatement *node) override;
  void VisitFunctionParameterDeclaration(
      const FunctionParameterDeclaration *node) override;
  void VisitFunctionDeclaration(const FunctionDeclaration *node) override;
  void VisitBinaryExpression(const BinaryExpression *node) override;
  void VisitTernaryExpression(const TernaryExpression *node) override;
  void
  VisitStructMemberReference(const StructMemberReference *node) override;
  void
  VisitStructInitExpression(const StructInitExpression *node) override;
  void VisitUnaryExpression(const UnaryExpression *node) override;
  void VisitCallExpression(const CallExpression *node) override;
  void VisitReferenceExpression(const ReferenceExpression *node) override;
  void VisitIntegerLiteralExpression(
      const IntegerLiteralExpression *node) override;
  void VisitFloatLiteralExpression(
      const FloatLiteralExpression *node) override;
  void VisitArrayExpression(const ArrayExpression *node) override;
  void VisitImplicitCastExpression(
      const ImplicitCastExpression *node) override;
  void VisitInitializerListExpression(
      const InitializerListExpression *node) override;
  void VisitTranslationUnit(const TranslationUnit *node) override;
};

#endif // AST_PRINT_HPP
