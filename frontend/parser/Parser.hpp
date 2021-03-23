#ifndef PARSER_H
#define PARSER_H

#include "../../middle_end/IR/IRFactory.hpp"
#include "../ast/AST.hpp"
#include "../lexer/Lexer.hpp"
#include "../lexer/Token.hpp"
#include "SymbolTable.hpp"
#include <memory>
#include <string>
#include <vector>

class Parser {
public:
  std::unique_ptr<Node> Parse();

  Parser() = delete;

  Parser(std::vector<std::string> &s, IRFactory *IRF) : lexer(s), IRF(IRF) {}

  Token Lex() { return lexer.Lex(); }

  Token::TokenKind GetCurrentTokenKind() {
    return lexer.GetCurrentToken().GetKind();
  }

  Token Expect(Token::TokenKind Token);

  /// Helper function to make insertion to the symbol table stack more compact
  /// and readable
  void InsertToSymTable(const std::string &SymName, ComplexType SymType,
                        const bool ToGlobal, ValueType SymValue);

  std::unique_ptr<Node> ParseTranslationUnit();
  std::unique_ptr<Node> ParseExternalDeclaration();
  std::unique_ptr<FunctionDeclaration>
  ParseFunctionDeclaration(const Type &ReturnType, const Token &Name);
  std::unique_ptr<VariableDeclaration> ParseVaraibleDeclaration();
  Node ParseReturnTypeSpecifier();
  std::vector<std::unique_ptr<FunctionParameterDeclaration>>
  ParseParameterList();
  std::unique_ptr<FunctionParameterDeclaration> ParseParameterDeclaration();
  Type ParseTypeSpecifier();
  std::unique_ptr<CompoundStatement> ParseCompoundStatement();
  std::unique_ptr<ReturnStatement> ParseReturnStatement();
  std::unique_ptr<Statement> ParseStatement();
  std::unique_ptr<ExpressionStatement> ParseExpressionStatement();
  std::unique_ptr<Expression> ParseExpression();
  std::unique_ptr<Expression> ParseBinaryExpression();
  std::unique_ptr<Expression>
  ParseBinaryExpressionRHS(int Precedence, std::unique_ptr<Expression> LHS);
  std::unique_ptr<Expression> ParseIdentifierExpression();
  std::unique_ptr<Expression> ParsePrimaryExpression();
  std::unique_ptr<WhileStatement> ParseWhileStatement();
  std::unique_ptr<ForStatement> ParseForStatement();
  std::unique_ptr<IfStatement> ParseIfStatement();
  std::unique_ptr<Expression> ParseConstantExpression();
  unsigned ParseIntegerConstant();
  double ParseRealConstant();

private:
  Lexer lexer;
  SymbolTableStack SymTabStack;
  IRFactory *IRF;
};

#endif