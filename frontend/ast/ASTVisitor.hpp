#ifndef AST_VISITOR_HPP
#define AST_VISITOR_HPP

class VariableDeclaration;
class MemberDeclaration;
class StructDeclaration;
class EnumDeclaration;
class CompoundStatement;
class ExpressionStatement;
class IfStatement;
class SwitchStatement;
class WhileStatement;
class ForStatement;
class ReturnStatement;
class BreakStatement;
class ContinueStatement;
class FunctionParameterDeclaration;
class FunctionDeclaration;
class BinaryExpression;
class TernaryExpression;
class StructMemberReference;
class StructInitExpression;
class UnaryExpression;
class CallExpression;
class ReferenceExpression;
class IntegerLiteralExpression;
class FloatLiteralExpression;
class ArrayExpression;
class ImplicitCastExpression;
class InitializerListExpression;
class TranslationUnit;

class ASTVisitor {
public:
  virtual void
  VisitVariableDeclaration(const VariableDeclaration *node) = 0;
  virtual void VisitMemberDeclaration(const MemberDeclaration *node) = 0;
  virtual void VisitStructDeclaration(const StructDeclaration *node) = 0;
  virtual void VisitEnumDeclaration(const EnumDeclaration *node) = 0;
  virtual void VisitCompoundStatement(const CompoundStatement *node) = 0;
  virtual void
  VisitExpressionStatement(const ExpressionStatement *node) = 0;
  virtual void VisitIfStatement(const IfStatement *node) = 0;
  virtual void VisitSwitchStatement(const SwitchStatement *node) = 0;
  virtual void VisitWhileStatement(const WhileStatement *node) = 0;
  virtual void VisitForStatement(const ForStatement *node) = 0;
  virtual void VisitReturnStatement(const ReturnStatement *node) = 0;
  virtual void VisitBreakStatement(const BreakStatement *node) = 0;
  virtual void VisitContinueStatement(const ContinueStatement *node) = 0;
  virtual void VisitFunctionParameterDeclaration(
      const FunctionParameterDeclaration *node) = 0;
  virtual void
  VisitFunctionDeclaration(const FunctionDeclaration *node) = 0;
  virtual void VisitBinaryExpression(const BinaryExpression *node) = 0;
  virtual void VisitTernaryExpression(const TernaryExpression *node) = 0;
  virtual void
  VisitStructMemberReference(const StructMemberReference *node) = 0;
  virtual void
  VisitStructInitExpression(const StructInitExpression *node) = 0;
  virtual void VisitUnaryExpression(const UnaryExpression *node) = 0;
  virtual void VisitCallExpression(const CallExpression *node) = 0;
  virtual void
  VisitReferenceExpression(const ReferenceExpression *node) = 0;
  virtual void
  VisitIntegerLiteralExpression(const IntegerLiteralExpression *node) = 0;
  virtual void
  VisitFloatLiteralExpression(const FloatLiteralExpression *node) = 0;
  virtual void VisitArrayExpression(const ArrayExpression *node) = 0;
  virtual void
  VisitImplicitCastExpression(const ImplicitCastExpression *node) = 0;
  virtual void VisitInitializerListExpression(
      const InitializerListExpression *node) = 0;
  virtual void VisitTranslationUnit(const TranslationUnit *node) = 0;
};

#endif // AST_VISITOR_HPP
