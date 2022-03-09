#ifndef PARSER_H
#define PARSER_H

#include "../../middle_end/IR/IRFactory.hpp"
#include "../ast/AST.hpp"
#include "../lexer/Lexer.hpp"
#include "../lexer/Token.hpp"
#include "../ErrorLogger.hpp"
#include "SymbolTable.hpp"
#include <memory>
#include <string>
#include <vector>

class Parser {
public:
  std::unique_ptr<Node> Parse();

  Parser() = delete;

  Parser(std::vector<std::string> &s, IRFactory *IRF, ErrorLogger &EL)
      : lexer(s), IRF(IRF), ErrorLog(EL) {}

  Token Lex() { return lexer.Lex(); }

  Token GetCurrentToken() { return lexer.GetCurrentToken(); }

  Token::TokenKind GetCurrentTokenKind() {
    return lexer.GetCurrentToken().GetKind();
  }

  Token Expect(Token::TokenKind Token);

  /// Helper function to make insertion to the symbol table stack more compact
  /// and readable
  void InsertToSymTable(const std::string &SymName, Type SymType,
                        const bool ToGlobal, ValueType SymValue);

  bool IsUserDefined(std::string Name);
  Type GetUserDefinedType(std::string Name);
  std::vector<std::string> GetUserDefinedTypeMembers(std::string Name);

  unsigned ParseQualifiers();
  Type ParseType(Token::TokenKind tk);
  bool IsTypeSpecifier(Token T);
  bool IsReturnTypeSpecifier(Token T);
  void ParseArrayDimensions(Type &type);
  bool IsQualifiedType(Token T);

  std::unique_ptr<Node> ParseTranslationUnit();
  std::unique_ptr<Node> ParseExternalDeclaration();
  std::unique_ptr<FunctionDeclaration>
  ParseFunctionDeclaration(const Type &ReturnType, const Token &Name);
  std::unique_ptr<VariableDeclaration> ParseVariableDeclaration(Type type);
  std::vector<std::unique_ptr<Statement>> ParseVariableDeclarationList();
  std::unique_ptr<MemberDeclaration> ParseMemberDeclaration();
  std::unique_ptr<StructDeclaration> ParseStructDeclaration(unsigned Qualifiers);
  std::unique_ptr<EnumDeclaration> ParseEnumDeclaration(unsigned Qualifiers);
  Node ParseReturnTypeSpecifier();
  std::vector<std::unique_ptr<FunctionParameterDeclaration>>
  ParseParameterList(bool &HasVarArg);
  std::unique_ptr<FunctionParameterDeclaration> ParseParameterDeclaration();
  Type ParseTypeSpecifier();
  std::unique_ptr<CompoundStatement> ParseCompoundStatement();
  std::unique_ptr<ReturnStatement> ParseReturnStatement();
  std::unique_ptr<BreakStatement> ParseBreakStatement();
  std::unique_ptr<ContinueStatement> ParseContinueStatement();
  std::unique_ptr<Statement> ParseStatement();
  std::unique_ptr<ExpressionStatement> ParseExpressionStatement();
  std::unique_ptr<Expression> ParseExpression();
  std::unique_ptr<Expression> ParsePostFixExpression();
  std::unique_ptr<Expression> ParseUnaryExpression();
  std::unique_ptr<Expression> ParseBinaryExpression();
  std::unique_ptr<Expression>
  ParseTernaryExpression(std::unique_ptr<Expression> Condition);
  std::unique_ptr<Expression>
  ParseBinaryExpressionRHS(int Precedence, std::unique_ptr<Expression> LHS);
  std::unique_ptr<Expression> ParseCallExpression(Token ID);
  std::unique_ptr<Expression> ParseArrayExpression(std::unique_ptr<Expression> Base);
  std::unique_ptr<Expression> ParseIdentifierExpression();
  std::unique_ptr<Expression> ParsePrimaryExpression();
  std::unique_ptr<Expression> ParseInitializerListExpression();
  std::unique_ptr<WhileStatement> ParseWhileStatement();
  std::unique_ptr<DoWhileStatement> ParseDoWhileStatement();
  std::unique_ptr<ForStatement> ParseForStatement();
  std::unique_ptr<IfStatement> ParseIfStatement();
  std::unique_ptr<SwitchStatement> ParseSwitchStatement();
  std::unique_ptr<Expression> ParseConstantExpression();
  unsigned ParseIntegerConstant();
  double ParseRealConstant();

  ErrorLogger &GetErrorLog() { return ErrorLog; }

private:
  Lexer lexer;
  SymbolTableStack SymTabStack;
  IRFactory *IRF;

  /// Type name to type, and the list of names for the struct field
  std::map<std::string, std::tuple<Type, std::vector<std::string>>> UserDefinedTypes;

  /// Mapping identifiers to types. Eg: "typedef int i32" -> {"i32", Type::Int}
  std::map<std::string, Type> TypeDefinitions;

  /// Used for determining if implicit cast need or not in return statements
  Type CurrentFuncRetType = Type::Invalid;

  /// The amount of return seen in the current function being parsed
  unsigned ReturnsNumber = 0;

  ErrorLogger &ErrorLog;
};

#endif