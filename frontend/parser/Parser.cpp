#include "Parser.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <typeinfo>

Token Parser::Expect(Token::TokenKind TKind) {
  auto t = Lex();
  if (t.GetKind() != TKind) {
    std::cout << ":" << t.GetLineNum() + 1 << ":" << t.GetColNum() + 1
              << ": error: Unexpected symbol `" << t.GetString()
              << "`. Expected is `" << Token::ToString(TKind) << "`."
              << std::endl
              << "\t\t" << lexer.GetSource()[t.GetLineNum()] << std::endl
              << std::endl;
  }
  return t; // consume Tokens
}

std::unique_ptr<Node> Parser::Parse() {
  return std::move(ParseExternalDeclaration());
}

// TODO: To report the location of error we would need the token holding the
// symbol name. It would be wise anyway to save it in AST nodes rather than
// just a string.
void Parser::InsertToSymTable(const std::string &SymName, Type SymType,
                              const bool ToGlobal = false,
                              ValueType SymValue = ValueType()) {

  std::tuple<std::string, Type, ValueType> SymEntry(SymName, SymType,
                                                           SymValue);
  // Check if it is already defined in the current scope
  if (SymTabStack.ContainsInCurrentScope(SymEntry))
    std::cout << "error: Symbol '" + SymName + "' with type '" +
                     SymType.ToString() + "' is already defined."
              << std::endl
              << std::endl;
  else if (ToGlobal)
    SymTabStack.InsertGlobalEntry(SymEntry);
  else
    SymTabStack.InsertEntry(SymEntry);
}

void static UndefinedSymbolError(Token sym, Lexer &L) {
  std::cout << ":" << sym.GetLineNum() + 1 << ":" << sym.GetColNum() + 1
            << ": error: "
            << "Undefined symbol '" << sym.GetString() << "'." << std::endl
            << "\t\t" << L.GetSource()[sym.GetLineNum()].substr(sym.GetColNum())
            << std::endl
            << std::endl;
}

void static ArrayTypeMismatchError(Token sym, Type actual) {
  std::cout << sym.GetLineNum() + 1 << ":" << sym.GetColNum() + 1 << " error:"
            << ": Type mismatch'" << sym.GetString() << "' type is '"
            << actual.ToString() << "', it is not an array type.'" << std::endl;
}

void static EmitError(const std::string &msg, Lexer &L) {
  std::cout << ":" << L.GetLineNum() << ": error: " << msg << std::endl
            << "\t\t" << L.GetSource()[L.GetLineNum() - 1] << std::endl
            << std::endl;
}

void static EmitError(const std::string &msg, Lexer &L, Token &T) {
  std::cout << ":" << T.GetLineNum() + 1 << ":" << T.GetColNum() + 1
            << ": error: " << msg << std::endl
            << "\t\t" << L.GetSource()[T.GetLineNum()] << std::endl
            << std::endl;
}

bool Parser::IsTypeSpecifier(Token::TokenKind tk) {
  switch (tk) {
  case Token::Char:
  case Token::Int:
  case Token::Double:
  case Token::Struct:
    return true;

  default:
    break;
  }
  return false;
}

static bool IsUnaryOperator(Token::TokenKind tk) {
  switch (tk) {
  case Token::Astrix:
    return true;

  default:
    break;
  }
  return false;
}

bool Parser::IsReturnTypeSpecifier(Token::TokenKind tk) {
  return tk == Token::Void || IsTypeSpecifier(tk);
}

Type Parser::ParseType(Token::TokenKind tk) {
  Type Result;

  switch (tk) {
  case Token::Void:
    Result.SetTypeVariant(Type::Void);
    break;
  case Token::Char:
    Result.SetTypeVariant(Type::Char);
    break;
  case Token::Int:
    Result.SetTypeVariant(Type::Int);
    break;
  case Token::Double:
    Result.SetTypeVariant(Type::Double);
    break;
  case Token::Struct: {
    Lex(); // eat 'struct' here
    auto CurrToken = lexer.GetCurrentToken();

    std::string Name = CurrToken.GetString();
    Result = std::get<0>(UserDefinedTypes[Name]);
    break;
  }
  default:
    assert(!"Unknown token kind.");
    break;
  }

  return Result;
}

// <ExternalDeclaration> ::= <FunctionDeclaration>
//                         | <VariableDeclaration>
//
// First set : {void, int, double}
// Second set : {Identifier}
std::unique_ptr<Node> Parser::ParseExternalDeclaration() {
  std::unique_ptr<TranslationUnit> TU = std::make_unique<TranslationUnit>();
  auto TokenKind = GetCurrentTokenKind();

  while (IsReturnTypeSpecifier(TokenKind) || lexer.Is(Token::Struct) ||
      lexer.Is(Token::Enum)) {
    if (lexer.Is(Token::Struct) && lexer.LookAhead(3).GetKind() == Token::LeftCurly) {
      TU->AddDeclaration(ParseStructDeclaration());
      TokenKind = GetCurrentTokenKind();
      continue;
    }

    if (lexer.Is(Token::Enum)) {
      TU->AddDeclaration(ParseEnumDeclaration());
      TokenKind = GetCurrentTokenKind();
      continue;
    }

    Type type = ParseType(TokenKind);
    CurrentFuncRetType = type;
    Lex();

    auto Name = Expect(Token::Identifier);
    auto NameStr = Name.GetString();

    // if a function declaration then a left parenthesis '(' expected
    if (lexer.Is(Token::LeftParen)) {
      TU->AddDeclaration(ParseFunctionDeclaration(type, Name));
    } else { // Variable declaration
      std::vector<unsigned> Dimensions;

      // array declaration
      while (lexer.Is(Token::LeftBracet)) {
        Lex(); // consume '['
        Dimensions.push_back(ParseIntegerConstant());
        Expect(Token::RightBracet);
        if (lexer.IsNot(Token::Comma))
          break;
        Lex(); // consume ','
      }

      Expect(Token::SemiColon);

      InsertToSymTable(NameStr, Type(type, Dimensions));

      TU->AddDeclaration(
          std::make_unique<VariableDeclaration>(NameStr, type, Dimensions));
    }

    TokenKind = GetCurrentTokenKind();
  }

  return TU;
}

// <FunctionDeclaration> ::= <ReturnTypeSpecifier> <Identifier> '('
//                             <ParameterList>? ')' ';'
//			   | <ReturnTypeSpecifier> <Identifier>
//                             '(' <ParameterList>? ')' <CompoundStatement>
std::unique_ptr<FunctionDeclaration>
Parser::ParseFunctionDeclaration(const Type &ReturnType, const Token &Name) {
  Expect(Token::LeftParen); // consume '('

  // Creating new scope by pushing a new symbol table to the stack
  SymTabStack.PushSymTable();

  auto PL = ParseParameterList();

  Expect(Token::RightParen);

  auto FuncType = FunctionDeclaration::CreateType(ReturnType, PL);
  auto NameStr = Name.GetString();
  InsertToSymTable(NameStr, FuncType, true);

  // For now assume that a function is defined always not just declared
  // TODO: was it planed to have function declaration?
  auto Body = ParseCompoundStatement();

  // Removing the function's scope since we done with its parsing
  SymTabStack.PopSymTable();

  return std::make_unique<FunctionDeclaration>(FuncType, NameStr, PL, Body);
}

// <ParameterList> ::= <ParameterDeclaration>? {',' <ParameterDeclaration>}*
std::vector<std::unique_ptr<FunctionParameterDeclaration>>
Parser::ParseParameterList() {
  std::vector<std::unique_ptr<FunctionParameterDeclaration>> Params;

  if (!IsTypeSpecifier(GetCurrentTokenKind()))
    return Params;

  Params.push_back(ParseParameterDeclaration());
  while (lexer.Is(Token::Comma)) {
    Lex(); // consume ','
    Params.push_back(ParseParameterDeclaration());
  }
  return Params;
}

// <ParameterDeclaration> ::= { <TypeSpecifier> '*'* <Identifier>? }?
std::unique_ptr<FunctionParameterDeclaration>
Parser::ParseParameterDeclaration() {
  std::unique_ptr<FunctionParameterDeclaration> FPD =
      std::make_unique<FunctionParameterDeclaration>();

  if (IsTypeSpecifier(GetCurrentTokenKind())) {
    Type type = ParseTypeSpecifier();
    Lex();

    while (lexer.Is(Token::Astrix)) {
      type.IncrementPointerLevel();
      Lex(); // Eat the * character
    }

    FPD->SetType(type);

    if (lexer.Is(Token::Identifier)) {
      auto Name = Lex().GetString();
      FPD->SetName(Name);
      InsertToSymTable(Name, type);
    }
  }

  return FPD;
}

// TODO: Extend comment
// <TypeSpecifier> ::= int
//                   | double
Type Parser::ParseTypeSpecifier() {
  auto TokenKind = GetCurrentTokenKind();

  if (!IsTypeSpecifier(TokenKind))
    assert(!"Invalid type");

  return ParseType(TokenKind);
}

// <CompoundStatement> ::= '{' <VariableDeclaration>* <Statement>* '}'
std::unique_ptr<CompoundStatement> Parser::ParseCompoundStatement() {
  Expect(Token::LeftCurly);

  std::vector<std::unique_ptr<VariableDeclaration>> Declarations;

  while (IsTypeSpecifier(GetCurrentTokenKind()))
    Declarations.push_back(std::move(ParseVariableDeclaration()));

  std::vector<std::unique_ptr<Statement>> Statements;

  while (lexer.IsNot(Token::RightCurly))
    Statements.push_back(std::move(ParseStatement()));

  Expect(Token::RightCurly);

  return std::make_unique<CompoundStatement>(Declarations, Statements);
}

// <VariableDeclaration> ::= <TypeSpecifier> '*'* <Identifier>
//                           {'[' <IntegerConstant> ]'}* { = <Expression> }? ';'
std::unique_ptr<VariableDeclaration> Parser::ParseVariableDeclaration() {
  Type type = ParseTypeSpecifier();
  Lex();

  while (lexer.Is(Token::Astrix)) {
    type.IncrementPointerLevel();
    Lex(); // Eat the * character
  }

  std::string Name = Expect(Token::Identifier).GetString();

  std::vector<unsigned> Dimensions;
  while (lexer.Is(Token::LeftBracet)) {
    Lex();
    Dimensions.push_back(ParseIntegerConstant());
    Expect(Token::RightBracet);
  }

  if (!Dimensions.empty())
    type = Type(type, Dimensions);

  InsertToSymTable(Name, type);

  // If the variable initialized
  std::unique_ptr<Expression> InitExpr = nullptr;
  if (lexer.Is(Token::Equal)) {
    Lex(); // eat '='
    InitExpr = ParseExpression();
  }

  Expect(Token::SemiColon);

  auto VD = std::make_unique<VariableDeclaration>(Name, type, Dimensions);

  if (InitExpr)
    VD->SetInitExpr(std::move(InitExpr));

  return VD;
}

// <VariableDeclaration> ::= <TypeSpecifier> '*'* <Identifier>
//                           {'[' <IntegerConstant> ]'}* ';'
std::unique_ptr<MemberDeclaration> Parser::ParseMemberDeclaration() {
  Type type = ParseTypeSpecifier();
  Lex();

  while (lexer.Is(Token::Astrix)) {
    type.IncrementPointerLevel();
    Lex(); // Eat the * character
  }

  std::string Name = Expect(Token::Identifier).GetString();

  std::vector<unsigned> Dimensions;
  while (lexer.Is(Token::LeftBracet)) {
    Lex();
    Dimensions.push_back(ParseIntegerConstant());
    Expect(Token::RightBracet);
  }

  Expect(Token::SemiColon);

  return std::make_unique<MemberDeclaration>(Name, type, Dimensions);
}

// <StructDeclaration> ::= 'struct' <Identifier>
//                                  '{' <StructDeclarationList>+ '}' ';'
std::unique_ptr<StructDeclaration> Parser::ParseStructDeclaration() {
  Expect(Token::Struct);

  std::string Name = Expect(Token::Identifier).GetString();

  Expect(Token::LeftCurly);

  std::vector<std::unique_ptr<MemberDeclaration>> Members;
  Type type(Type::Struct);
  type.SetName(Name);

  std::vector<std::string> StructMemberIdentifiers;
  while (lexer.IsNot(Token::RightCurly)) {
    auto MD = ParseMemberDeclaration();
    type.GetTypeList().push_back(MD->GetType());
    StructMemberIdentifiers.push_back(MD->GetName());
    Members.push_back(std::move(MD));
  }

  Expect(Token::RightCurly);
  Expect(Token::SemiColon);

  // saving the struct type and name
  UserDefinedTypes[Name] = {type, std::move(StructMemberIdentifiers)};

  return std::make_unique<StructDeclaration>(Name, Members, type);
}

// <EnumDeclaration> ::= 'enum' '{' <Identifier> (, <Identifier>)* '}' ';'
std::unique_ptr<EnumDeclaration> Parser::ParseEnumDeclaration() {
  Expect(Token::Enum);
  Expect(Token::LeftCurly);

  EnumDeclaration::EnumList Enumerators;

  int EnumCounter = 0;
  do {
    if (lexer.Is(Token::Comma))
        Lex();

    auto Identifier = Expect(Token::Identifier);
    Enumerators.push_back({Identifier.GetString(), EnumCounter});

    // Insert into the symbol table and for now assign the index of the enum
    // to it, not considering explicit assignments like "enum { A = 10 };"
    InsertToSymTable(Identifier.GetString(), Type(Type::Int),
                     false, ValueType((unsigned)EnumCounter));
    EnumCounter++;
  } while (lexer.Is(Token::Comma));

  Expect(Token::RightCurly);
  Expect(Token::SemiColon);

  return std::make_unique<EnumDeclaration>(Enumerators);
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
//               | <ForStatement>
//               | <IfStatement>
//               | <SwitchStatement>
//               | <CompoundStatement>
//               | <ReturnStatement>
std::unique_ptr<Statement> Parser::ParseStatement() {
  if (lexer.Is(Token::If))
    return ParseIfStatement();
  if (lexer.Is(Token::Switch))
    return ParseSwitchStatement();
  if (lexer.Is(Token::While))
    return ParseWhileStatement();
  if (lexer.Is(Token::For))
    return ParseForStatement();
  if (lexer.Is(Token::LeftCurly))
    return ParseCompoundStatement();
  if (lexer.Is(Token::Break))
    return ParseBreakStatement();
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

// <SwitchStatement> ::= switch '(' <Expression> ')' '{'
//                       'case' <Constant> ':' <Statement>*
//                       'default' ':' <Statement>*
//                       '}'
std::unique_ptr<SwitchStatement> Parser::ParseSwitchStatement() {
  std::unique_ptr<SwitchStatement> SS = std::make_unique<SwitchStatement>();

  Expect(Token::Switch);
  Expect(Token::LeftParen);
  SS->SetCondition(std::move(ParseExpression()));
  Expect(Token::RightParen);
  Expect(Token::LeftCurly);

  SwitchStatement::VecOfCasesData CasesData;

  unsigned FoundDefaults = 0;

  while (lexer.Is(Token::Case) || lexer.Is(Token::Default)) {
    const bool IsCase = lexer.Is(Token::Case);
    Lex(); // eat 'case' or 'default'

    std::unique_ptr<Expression> ConstExpr;

    if (IsCase) {
      ConstExpr = ParseConstantExpression();
      // For now if its not a constant then assuming its an enum const which is
      // an identifier currently handled by the below function
      // TODO: Move the handling of enum const to ParseConstantExpression maybe
      // but, this would also need some caution
      if (!ConstExpr)
        ConstExpr = ParseIdentifierExpression();
      // TODO: make it a semantic check and not assertion
      assert(ConstExpr.get()->GetResultType().IsIntegerType() &&
             "Case expression must be an integer type");
    }

    Expect(Token::Colon);

    SwitchStatement::VecOfStmts Statements;
    while (lexer.IsNot(Token::RightCurly) && lexer.IsNot(Token::Case) &&
           lexer.IsNot(Token::Default))
      Statements.push_back(std::move(ParseStatement()));

    if (IsCase) {
      int CaseConstVal = dynamic_cast<IntegerLiteralExpression*>(ConstExpr.get())->GetSIntValue();
      CasesData.push_back({CaseConstVal, std::move(Statements)});
    } else {
      FoundDefaults++;
      // TODO: Make it a semantic check
      assert(FoundDefaults <= 1 && "Too much default case!");
      SS->SetDefaultBody(std::move(Statements));
    }
  }
  
  SS->SetCaseBodies(std::move(CasesData));

  Expect(Token::RightCurly);

  return SS;
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

// <ForStatement> ::= for '(' <Expression> ')' <Statement>
std::unique_ptr<ForStatement> Parser::ParseForStatement() {
  std::unique_ptr<ForStatement> FS = std::make_unique<ForStatement>();

  Expect(Token::For);
  Expect(Token::LeftParen);
  FS->SetInit(std::move(ParseExpression()));
  Expect(Token::SemiColon);
  FS->SetCondition(std::move(ParseExpression()));
  Expect(Token::SemiColon);
  FS->SetIncrement(std::move(ParseExpression()));
  Expect(Token::RightParen);
  FS->SetBody(std::move(ParseStatement()));

  return FS;
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

// <BreakStatement> ::= 'break' ';'
std::unique_ptr<BreakStatement> Parser::ParseBreakStatement() {
  Expect(Token::Break);
  Expect(Token::SemiColon);
  return std::make_unique<BreakStatement>();
}

// <ReturnStatement> ::= return <Expression>? ';'
// TODO: we need explicit type conversions here as well
std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
  Expect(Token::Return);
  auto Expr = ParseExpression();
  auto LeftType = CurrentFuncRetType.GetTypeVariant();
  auto RightType = Expr->GetResultType().GetTypeVariant();
  std::unique_ptr<ReturnStatement> RS;

  if (LeftType != RightType) {
    std::unique_ptr<Expression> CastExpr =
        std::make_unique<ImplicitCastExpression>(std::move(Expr), LeftType);
    RS = std::make_unique<ReturnStatement>(std::move(CastExpr));
  } else {
    RS = std::make_unique<ReturnStatement>(std::move(Expr));
  }

  Expect(Token::SemiColon);
  return RS;
}

// <Expression> ::= <AssignmentExpression>
std::unique_ptr<Expression> Parser::ParseExpression() {
  return ParseBinaryExpression();
}

static bool IsPostfixOperator(Token tk) {
  switch (tk.GetKind()) {
  case Token::PlusPlus:
  case Token::MinusMinus:
  case Token::LeftParen:
  case Token::LeftBracet:
  case Token::Dot:
    return true;

  default:
    break;
  }
  return false;
}

// <PostFixExpression> ::= <PrimaryExpression>
//                       | <PostFixExpression> '++'
//                       | <PostFixExpression> '--'
//                       | <PostFixExpression> '(' <Arguments> ')'
//                       | <PostFixExpression> '[' <Expression> ']'
//                       | <PostFixExpression> '.' <Identifier>
std::unique_ptr<Expression> Parser::ParsePostFixExpression() {
  auto CurrentToken = lexer.GetCurrentToken();
  auto Expr = ParsePrimaryExpression();
  assert(Expr && "Cannot be NULL");

  while (IsPostfixOperator(lexer.GetCurrentToken())) {
    if (lexer.Is(Token::PlusPlus) || lexer.Is(Token::MinusMinus)) {
      auto Operation = lexer.GetCurrentToken();
      Lex(); // eat the token
      Expr->SetLValueness(true);
      Expr = std::make_unique<UnaryExpression>(Operation, std::move(Expr));
    }
    // Parse a CallExpression here
    else if (lexer.Is(Token::LeftParen)) {
      Expr = ParseCallExpression(CurrentToken);
    }
    // parse ArrayExpression
    else if (lexer.Is(Token::LeftBracet)) {
      Expr = ParseArrayExpression(std::move(Expr));
    }
    // parse StructMemberAccess
    else if (lexer.Is(Token::Dot)) {
      Lex(); // eat the Dot
      auto MemberId = Expect(Token::Identifier);
      auto MemberIdStr = MemberId.GetString();

      assert(Expr->GetResultType().IsStruct() && "TODO: emit error");
      // find the type of the member
      auto StructDataTuple = UserDefinedTypes[Expr->GetResultType().GetName()];
      auto StructType = std::get<0>(StructDataTuple);
      auto StructMemberNames = std::get<1>(StructDataTuple);

      size_t i = 0;
      for (; i < StructMemberNames.size(); i++)
        if (StructMemberNames[i] == MemberIdStr)
          break;

      assert(i <= StructMemberNames.size() && "Member not found");

      Expr = std::make_unique<StructMemberReference>(std::move(Expr),
                                                     MemberIdStr, i);
    }
  }

  return Expr;
}

std::unique_ptr<Expression> Parser::ParseUnaryExpression() {
  auto UnaryOperation = lexer.GetCurrentToken();

  if (!IsUnaryOperator(UnaryOperation.GetKind()))
    return ParsePostFixExpression();

  Lex(); // eat the unary operation char

  std::unique_ptr<Expression> Expr;

  if (IsUnaryOperator((GetCurrentTokenKind())))
    return std::make_unique<UnaryExpression>(UnaryOperation,
                                             std::move(ParseUnaryExpression()));

  // TODO: Add semantic check that only pointer types are dereferenced
  Expr = ParsePostFixExpression();
  return std::make_unique<UnaryExpression>(UnaryOperation,
                                           std::move(Expr));
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
  auto LeftExpression = ParseUnaryExpression();
  assert(LeftExpression && "Cannot be NULL");

  return ParseBinaryExpressionRHS(0, std::move(LeftExpression));
}

std::unique_ptr<Expression>
Parser::ParseBinaryExpressionRHS(int Precedence,
                                 std::unique_ptr<Expression> LeftExpression) {
  while (true) {
    int TokenPrecedence = GetBinOpPrecedence(GetCurrentTokenKind());

    if (TokenPrecedence < Precedence)
      return LeftExpression;

    Token BinaryOperator = Lex();

    auto RightExpression = ParseUnaryExpression();

    // In case of an assignment check if the left operand since it should be an
    // lvalue. Which is either an identifier reference or an array expression.
    if (BinaryOperator.GetKind() == Token::Equal &&
        !dynamic_cast<ReferenceExpression *>(LeftExpression.get()) &&
        !dynamic_cast<ArrayExpression *>(LeftExpression.get()) &&
        !dynamic_cast<StructMemberReference *>(LeftExpression.get()))
      // TODO: Since now we have ImplicitCast nodes we have to either check if
      // the castable object is an lv....
      EmitError("lvalue required as left operand of assignment", lexer,
                BinaryOperator);

    //  If it is an equal binop (assignment) and the left hand side is an array
    // expression or reference expression, then it is an LValue.

    // FIX-ME: Should be solved in a better way. Seems like LLVM using
    // ImplicitCast for this purpose as well. Should investigate that solution.
    if (BinaryOperator.GetKind() == Token::Equal) {
      if (auto LE = dynamic_cast<ArrayExpression *>(LeftExpression.get()))
        LE->SetLValueness(true);
      else if (auto LE =
                   dynamic_cast<ReferenceExpression *>(LeftExpression.get()))
        LE->SetLValueness(true);
      else if (auto LE =
          dynamic_cast<StructMemberReference *>(LeftExpression.get()))
        LE->SetLValueness(true);
    }

    int NextTokenPrec = GetBinOpPrecedence(GetCurrentTokenKind());

    int Associviaty = 1; // left
    if (BinaryOperator.GetKind() == Token::Equal) {
      Associviaty = 0; // right
      NextTokenPrec++;
    }
    if (TokenPrecedence < NextTokenPrec)
      RightExpression = ParseBinaryExpressionRHS(TokenPrecedence + Associviaty,
                                                 std::move(RightExpression));

    // Implicit cast insertion if needed.
    auto LeftType = LeftExpression->GetResultType().GetTypeVariant();
    auto RightType = RightExpression->GetResultType().GetTypeVariant();

    // if its a modulo operation
    if (BinaryOperator.GetKind() == Token::Percent) {
      // then both side should be an integer type without a casting
      if (LeftType != Type::Int || RightType != Type::Int)
        EmitError("Modulo operator can only operate on integers", lexer,
                  BinaryOperator);
    }
    // Having different types.
    else if (LeftType != RightType) {
      // if an assingment, then try to cast the right hand side to type of the
      // left hand side
      if (BinaryOperator.GetKind() == Token::Equal) {
        if (!Type::IsImplicitlyCastable(RightType, LeftType))
          EmitError("Type mismatch", lexer, BinaryOperator);
        else {
          RightExpression = std::make_unique<ImplicitCastExpression>(
              std::move(RightExpression), LeftType);
        }
      }
      // Otherwise cast the one with lower conversion rank to the higher one
      else {
        auto DesiredType =
            Type::GetStrongestType(LeftType, RightType).GetTypeVariant();

        // If left hand side needs the conversion
        if (LeftType != DesiredType)
          LeftExpression = std::make_unique<ImplicitCastExpression>(
              std::move(LeftExpression), DesiredType);
        else // if the right one
          RightExpression = std::make_unique<ImplicitCastExpression>(
              std::move(RightExpression), DesiredType);
      }
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
  } else if (lexer.Is(Token::Real) || lexer.Is(Token::Integer)) {
    return ParseConstantExpression();
  } else {
    return nullptr;
  }
}

// <ConstantExpression> ::= [1-9][0-9]*
//                        | [0-9]+.[0-9]+
std::unique_ptr<Expression> Parser::ParseConstantExpression() {
  // Handle enumerator constant cases
  if (lexer.Is(Token::Identifier)) {
    auto Id = Expect(Token::Identifier);
    auto IdStr = Id.GetString();

    if (auto SymEntry = SymTabStack.Contains(IdStr)) {
      if (auto Val = std::get<2>(SymEntry.value()); !Val.IsEmpty())
        return std::make_unique<IntegerLiteralExpression>(Val.GetIntVal());
      else
        assert(!"Not an enumerator constant");
    } else
      assert(!"Not an enumerator constant");
  } else if (lexer.Is(Token::Integer))
    return std::make_unique<IntegerLiteralExpression>(ParseIntegerConstant());
  else
    return std::make_unique<FloatLiteralExpression>(ParseRealConstant());
}

std::unique_ptr<Expression> Parser::ParseCallExpression(Token Id) {
  // FIXME: Make this a semantic check
  assert(Id.GetKind() == Token::Identifier && "Identifier expected");
  Lex(); // eat the '('

  Type FuncType;

  if (auto SymEntry = SymTabStack.Contains(Id.GetString()))
    FuncType = std::get<1>(SymEntry.value());
  else
    UndefinedSymbolError(Id, lexer);

  std::vector<std::unique_ptr<Expression>> CallArgs;

  if (lexer.IsNot(Token::RightParen))
    CallArgs.push_back(ParseExpression());

  while (lexer.Is(Token::Comma)) {
    Lex();
    CallArgs.push_back(ParseExpression());
  }

  // Currently a function without argument is actually a function with
  // a type of ...(void), which is a special case checked first.
  auto FuncArgTypes = FuncType.GetArgTypes();
  auto FuncArgNum = FuncArgTypes.size();
  if (!(CallArgs.size() == 0 && FuncArgNum == 1 &&
        FuncArgTypes[0] == Type::Void)) {
    if (FuncArgNum != CallArgs.size())
      EmitError("arguments number mismatch", lexer);

    for (size_t i = 0; i < FuncArgNum; i++) {
      auto CallArgType = CallArgs[i]->GetResultType().GetTypeVariant();

      // If the ith argument type is not matching the expeccted one
      if (CallArgType != FuncArgTypes[i].GetTypeVariant()) {
        // Cast if allowed
        if (Type::IsImplicitlyCastable(CallArgType, FuncArgTypes[i].GetTypeVariant()))
          CallArgs[i] = std::make_unique<ImplicitCastExpression>(
              std::move(CallArgs[i]), FuncArgTypes[i]);
        else // otherwise its an error
          EmitError("argument type mismatch", lexer);
      }
    }
  }

  Expect(Token::RightParen);

  return std::make_unique<CallExpression>(Id.GetString(), CallArgs, FuncType);
}

std::unique_ptr<Expression> Parser::ParseArrayExpression(std::unique_ptr<Expression> Base) {
  Lex();
  auto IndexExpr = ParseExpression();
  Expect(Token::RightBracet);

  Type ActualType;

  // Type type = std::move(ActualType);
  Type type = Base->GetResultType();

  /// Remove the first N dimensions from the actual type. Example:
  /// ActualType is 'int arr[5][10]' and our reference is 'arr[0]'
  /// then the result type of 'arr[0]' is 'int[10]'. N is the
  /// amount of index expressions used when referencing the array here
  /// 'arr'. In the example its 1.
  type.GetDimensions().erase(type.GetDimensions().begin(),
                             type.GetDimensions().begin() +
                             1);

  Base->SetLValueness(true);
  return std::make_unique<ArrayExpression>(Base, IndexExpr, type);
}

// <IdentifierExpression> ::= Identifier
std::unique_ptr<Expression> Parser::ParseIdentifierExpression() {
  auto Id = Expect(Token::Identifier);

  // Identifier case
  auto RE = std::make_unique<ReferenceExpression>(Id);
  auto IdStr = Id.GetString();

  if (auto SymEntry = SymTabStack.Contains(IdStr)) {
    // If the symbol value is a know constant like in case of enumerators, then
    // return just a constant expression
    // TODO: Maybe do ths check earlier to save ourself from creating RE for
    // nothing
    if (auto Val = std::get<2>(SymEntry.value()); !Val.IsEmpty())
      return std::make_unique<IntegerLiteralExpression>(Val.GetIntVal());

    auto Type = std::get<1>(SymEntry.value());
    RE->SetType(Type);
  } else if (UserDefinedTypes.count(IdStr) > 0) {
    auto Type = std::get<0>(UserDefinedTypes[IdStr]);
    RE->SetType(std::move(Type));
  }
  else
    UndefinedSymbolError(Id, lexer);

  return RE;
}
