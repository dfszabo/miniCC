#include "Parser.hpp"
#include <cassert>
#include <iostream>
#include <memory>

Token Parser::Expect(Token::TokenKind Token) {
  if (lexer.IsNot(Token))
    std::cout << "Error: Expected other thing here, not this '"
              << lexer.GetCurrentToken().GetString() << "'." << std::endl;
  else
    return Lex(); // consume Tokens
}

std::unique_ptr<Node> Parser::Parse() {
  return std::move(ParseExternalDeclaration());
}

// TODO: To report the location of error we would need the token holding the
// symbol name. It would be wise anyway to save it in AST nodes rather than
// just a string.
void Parser::InsertToSymTable(const std::string &SymName, ComplexType SymType,
                              ValueType SymValue = ValueType()) {

  std::tuple<std::string, ComplexType, ValueType> SymEntry(SymName, SymType,
                                                           SymValue);
  // Check if it is already defined in the current scope
  if (SymTabStack.ContainsInCurrentScope(SymEntry))
    std::cout << "Error: Symbol '" + SymName + "' with type '" +
                     SymType.ToString() + "' is already defined."
              << std::endl
              << std::endl;
  else
    SymTabStack.InsertEntry(SymEntry);
}

static bool IsTypeSpecifier(Token::TokenKind tk) {
  return tk == Token::Int || tk == Token::Double;
}

static bool IsReturnTypeSpecifier(Token::TokenKind tk) {
  return tk == Token::Void || IsTypeSpecifier(tk);
}

static Type ParseType(Token::TokenKind tk) {
  Type Result;

  switch (tk) {
  case Token::Void:
    Result.SetTypeVariant(Type::Void);
    break;
  case Token::Int:
    Result.SetTypeVariant(Type::Int);
    break;
  case Token::Double:
    Result.SetTypeVariant(Type::Double);
    break;
  default:
    // TODO: emit error
    break;
  }

  return Result;
}

// <ExternalDeclaration> ::= <FunctionDeclaration>
//                         | <VaraibleDeclaration>
//
// First set : {void, int, double}
// Second set : {Identifier}
std::unique_ptr<Node> Parser::ParseExternalDeclaration() {
  std::unique_ptr<TranslationUnit> TU = std::make_unique<TranslationUnit>();
  auto TokenKind = GetCurrentTokenKind();

  while (IsReturnTypeSpecifier(TokenKind)) {
    Type Type = ParseType(TokenKind);
    Lex();

    auto Name = Expect(Token::Identifier);
    auto NameStr = Name.GetString();

    // if a function declaration then a left parenthesis '(' expected
    if (lexer.Is(Token::LeftParen)) {
      Lex(); // consume '('

      // Creating new scope by pushing a new symbol table to the stack
      SymTabStack.PushSymTable();

      auto PL = ParseParameterList();

      Expect(Token::RightParen);

      // for now assume that a function is defined always not just declared
      // TODO: was it planed to have function declaration?
      auto Body = ParseCompoundStatement();

      auto Function =
          std::make_unique<FunctionDeclaration>(Type, NameStr, PL, Body);

      // Removing the function's scope since we done with its parsing
      SymTabStack.PopSymTable();

      // FIXME: If we add the function to the symbol table here then its mean we
      // cannot call it from within this function (recursive call) since it get
      // only defined after parsing the function
      InsertToSymTable(NameStr, ComplexType(Function->GetType()));

      TU->AddDeclaration(std::move(Function));

    } else { // Variable declaration
      std::vector<unsigned> Dimensions;

      // array declaration
      while (lexer.Is(Token::LeftBracet)) {
        Lex(); // consume '['
        Dimensions.push_back(ParseIntegerConstant());
        Expect(Token::RightBracet);
        if (lexer.IsNot(Token::Colon))
          break;
        Lex(); // consume ','
      }

      Expect(Token::SemiColon);

      InsertToSymTable(NameStr, ComplexType(Type, Dimensions));

      TU->AddDeclaration(
          std::make_unique<VariableDeclaration>(NameStr, Type, Dimensions));
    }

    TokenKind = GetCurrentTokenKind();
  }

  return TU;
}

// <ParameterList> ::= <ParameterDeclaration>? {',' <ParameterDeclaration>}*
std::vector<std::unique_ptr<FunctionParameterDeclaration>>
Parser::ParseParameterList() {
  std::vector<std::unique_ptr<FunctionParameterDeclaration>> Params;

  if (!IsTypeSpecifier(GetCurrentTokenKind()))
    return Params;

  Params.push_back(ParseParameterDeclaration());
  while (lexer.Is(Token::Colon)) {
    Lex(); // consume ','
    Params.push_back(ParseParameterDeclaration());
  }
  return Params;
}

// <ParameterDeclaration> ::= { <TypeSpecifier> <Identifier>? }?
std::unique_ptr<FunctionParameterDeclaration>
Parser::ParseParameterDeclaration() {
  std::unique_ptr<FunctionParameterDeclaration> FPD =
      std::make_unique<FunctionParameterDeclaration>();

  if (IsTypeSpecifier(GetCurrentTokenKind())) {
    Type Type = ParseTypeSpecifier();
    Lex();
    FPD->SetType(Type);

    if (lexer.Is(Token::Identifier)) {
      auto Name = Lex().GetString();
      FPD->SetName(Name);
      InsertToSymTable(Name, ComplexType(Type));
    }
  }

  return FPD;
}

// <TypeSpecifier> ::= int
//                   | double
Type Parser::ParseTypeSpecifier() {
  auto TokenKind = GetCurrentTokenKind();

  if (!IsTypeSpecifier(TokenKind))
    ; // TODO: emit error

  return ParseType(TokenKind);
}

// <CompoundStatement> ::= '{' <VaraibleDeclaration>* <Statement>* '}'
std::unique_ptr<CompoundStatement> Parser::ParseCompoundStatement() {
  Expect(Token::LeftCurly);

  std::vector<std::unique_ptr<VariableDeclaration>> Declarations;

  while (IsTypeSpecifier(GetCurrentTokenKind()))
    Declarations.push_back(std::move(ParseVaraibleDeclaration()));

  std::vector<std::unique_ptr<Statement>> Statements;

  while (lexer.IsNot(Token::RightCurly))
    Statements.push_back(std::move(ParseStatement()));

  Expect(Token::RightCurly);

  return std::make_unique<CompoundStatement>(Declarations, Statements);
}

// <VaraibleDeclaration> ::= <TypeSpecifier> <Identifier>
//                           {'[' <IntegerConstant> ]'}* ';'
std::unique_ptr<VariableDeclaration> Parser::ParseVaraibleDeclaration() {
  Type Type = ParseTypeSpecifier();
  Lex();

  std::string Name = Expect(Token::Identifier).GetString();

  std::vector<unsigned> Dimensions;
  while (lexer.Is(Token::LeftBracet)) {
    Lex();
    Dimensions.push_back(ParseIntegerConstant());
    Expect(Token::RightBracet);
  }

  Expect(Token::SemiColon);

  InsertToSymTable(Name, ComplexType(Type, Dimensions));

  return std::make_unique<VariableDeclaration>(Name, Type, Dimensions);
}

unsigned Parser::ParseIntegerConstant() {
  Token T = Expect(Token::Integer);
  auto Str = T.GetString();
  unsigned Result = 0;

  for (auto c : Str) {
    Result *= 10;
    Result += c - '0';
  }
  return Result;
}

double Parser::ParseRealConstant() {
  Token T = Expect(Token::Real);
  auto Str = T.GetString();
  double WholePart = 0.0;

  for (auto c : Str) {
    if (c == '.')
      break;
    WholePart *= 10;
    WholePart += c - '0';
  }

  double FractionalPart = 0.0;
  unsigned Divider = 1;

  for (auto c : Str.substr(Str.find('.') + 1)) {
    FractionalPart += c - '0';
    Divider *= 10;
  }

  FractionalPart /= Divider;

  return WholePart + FractionalPart;
}

// <Statement> ::= <ExpressionStatement>
//               | <WhileStatement>
//               | <IfStatement>
//               | <CompoundStatement>
//               | <ReturnStatement>
std::unique_ptr<Statement> Parser::ParseStatement() {
  if (lexer.Is(Token::If))
    return ParseIfStatement();
  if (lexer.Is(Token::While))
    return ParseWhileStatement();
  if (lexer.Is(Token::LeftCurly))
    return ParseCompoundStatement();
  if (lexer.Is(Token::Return))
    return ParseReturnStatement();
  return ParseExpressionStatement();
}

// <IfStatement> ::= if '(' <Expression> ')' <Statement> {else <Statement>}?
std::unique_ptr<IfStatement> Parser::ParseIfStatement() {
  std::unique_ptr<IfStatement> IS = std::make_unique<IfStatement>();

  Expect(Token::If);
  Expect(Token::LeftParen);
  IS->SetCondition(std::move(ParseExpression()));
  Expect(Token::RightParen);
  IS->SetIfBody(std::move(ParseStatement()));

  if (lexer.Is(Token::Else)) {
    Lex();
    IS->SetElseBody(std::move(ParseStatement()));
  }
  return IS;
}

// <WhileStatement> ::= while '(' <Expression> ')' <Statement>
std::unique_ptr<WhileStatement> Parser::ParseWhileStatement() {
  std::unique_ptr<WhileStatement> WS = std::make_unique<WhileStatement>();

  Expect(Token::While);
  Expect(Token::LeftParen);
  WS->SetCondition(std::move(ParseExpression()));
  Expect(Token::RightParen);
  WS->SetBody(std::move(ParseStatement()));

  return WS;
}

// <ExpressionStatement> ::= <Expression>? ';'
std::unique_ptr<ExpressionStatement> Parser::ParseExpressionStatement() {
  std::unique_ptr<ExpressionStatement> ES =
      std::make_unique<ExpressionStatement>();

  if (lexer.IsNot(Token::SemiColon))
    ES->SetExpression(std::move(ParseExpression()));
  Expect(Token::SemiColon);

  return ES;
}

// <ReturnStatement> ::= return <Expression>? ';'
std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
  Expect(Token::Return);
  auto RS = std::make_unique<ReturnStatement>(ParseExpression());
  Expect(Token::SemiColon);
  return RS;
}

// <Expression> ::= <AssignmentExpression>
std::unique_ptr<Expression> Parser::ParseExpression() {
  return ParseBinaryExpression();
}

static int GetBinOpPrecedence(Token::TokenKind TK) {
  switch (TK) {
  case Token::Equal:
    return 10;
  case Token::DoubleAnd:
    return 20;
  case Token::And:
    return 30;
  case Token::DoubleEqual:
  case Token::BangEqual:
    return 40;
  case Token::LessThan:
  case Token::GreaterThan:
    return 50;
  case Token::Plus:
  case Token::Minus:
    return 60;
  case Token::Astrix:
  case Token::ForwardSlash:
  case Token::Percent:
    return 70;
  case Token::Bang:
    return 80;
  default:
    return -1;
  }
}

std::unique_ptr<Expression> Parser::ParseBinaryExpression() {
  auto LeftExpression = ParsePrimaryExpression();

  return ParseBinaryExpressionRHS(0, std::move(LeftExpression));
}

std::unique_ptr<Expression>
Parser::ParseBinaryExpressionRHS(int Precedence,
                                 std::unique_ptr<Expression> LeftExpression) {
  while (true) {
    int TokenPrecedence = GetBinOpPrecedence(GetCurrentTokenKind());

    if (TokenPrecedence < Precedence)
      return std::move(LeftExpression);

    Token BinaryOperator = Lex();

    auto RightExpression = ParsePrimaryExpression();

    int NextTokenPrec = GetBinOpPrecedence(GetCurrentTokenKind());
    if (TokenPrecedence < NextTokenPrec) {
      RightExpression = ParseBinaryExpressionRHS(TokenPrecedence + 1,
                                                 std::move(RightExpression));
    }

    LeftExpression = std::make_unique<BinaryExpression>(
        std::move(LeftExpression), BinaryOperator, std::move(RightExpression));
  }
}

// <PrimaryExpression> ::= <IdentifierExpression>
//                       | '(' <Expression> ')'
//                       | <ConstantExpression>
std::unique_ptr<Expression> Parser::ParsePrimaryExpression() {
  if (lexer.Is(Token::LeftParen)) {
    Lex();
    auto Expression = ParseExpression();
    Expect(Token::RightParen);
    return Expression;
  } else if (lexer.Is(Token::Identifier)) {
    return ParseIdentifierExpression();
  } else {
    return ParseConstantExpression();
  }
}

// <ConstantExpression> ::= [1-9][0-9]*
//                        | [0-9]+.[0-9]+
std::unique_ptr<Expression> Parser::ParseConstantExpression() {
  if (lexer.Is(Token::Integer))
    return std::make_unique<IntegerLiteralExpression>(ParseIntegerConstant());
  else
    return std::make_unique<FloatLiteralExpression>(ParseRealConstant());
}

// <IdentifierExpression> ::= Identifier
//                            {'(' {<Expression> {, <Expression>}* }?')'}?
//                          | Identifier {'[' <Expression> ']'}+
std::unique_ptr<Expression> Parser::ParseIdentifierExpression() {
  auto Id = Expect(Token::Identifier);

  if (lexer.IsNot(Token::LeftParen) && lexer.IsNot(Token::LeftBracet))
    return std::make_unique<ReferenceExpression>(Id);

  // Parse a CallExpression here
  if (lexer.Is(Token::LeftParen)) {
    Lex();

    std::vector<std::unique_ptr<Expression>> Args;

    if (lexer.IsNot(Token::RightParen))
      Args.push_back(ParseExpression());

    while (lexer.Is(Token::Colon)) {
      Lex();
      Args.push_back(ParseExpression());
    }

    Expect(Token::RightParen);

    return std::make_unique<CallExpression>(Id.GetString(), Args);
  }
  // parse ArrayExpression
  if (lexer.Is(Token::LeftBracet)) {
    Lex();
    std::vector<std::unique_ptr<Expression>> IndexExpressions;
    IndexExpressions.push_back(ParseExpression());
    Expect(Token::RightBracet);

    while (lexer.Is(Token::LeftBracet)) {
      Lex();
      IndexExpressions.push_back(ParseExpression());
      Expect(Token::RightBracet);
    }

    return std::make_unique<ArrayExpression>(Id, IndexExpressions);
  }
}
