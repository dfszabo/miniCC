#include "Parser.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <typeinfo>

const unsigned EMPTY_DIMENSION = ~0;

static bool IsUnsupported(const Token &T) {
  switch (T.GetKind()) {
  case Token::Alignas:
  case Token::Alignof:
  case Token::Atomic:
  case Token::Complex:
  case Token::Generic:
  case Token::Imaginary:
  case Token::Noreturn:
  case Token::StaticAssert:
  case Token::ThreadLocal:
    return true;

  default:
    return false;
  }
}

Token Parser::Expect(Token::TokenKind TKind) {
  auto t = GetCurrentToken();

  if (t.GetKind() != TKind) {
    if (t.GetKind() != Token::EndOfFile) {
      std::string Msg = "Unexpected symbol `" + t.GetString() +
                          "`, expected is `" + Token::ToString(TKind) + "`";
      ErrorLog.AddError(Msg, t);

      if (IsUnsupported(t)) {
        Msg = "'" + t.GetString() + "' is unsupported";
        ErrorLog.AddNote(Msg, t);
      }
    }
    else {
      std::string Error = "Reached the end of the file, but expected `" +
                          Token::ToString(TKind) + "`";
      ErrorLog.AddError(Error);
    }
    if (t.GetKind() == Token::Identifier)
      Lex();
  } else
    Lex(); // consume Tokens

  return t; // consume Tokens
}

std::unique_ptr<Node> Parser::Parse() {
  return std::move(ParseExternalDeclaration());
}

void Parser::InsertToSymTable(const Token &SymName, Type SymType,
                              const bool ToGlobal = false,
                              ValueType SymValue = ValueType()) {

  SymbolTableStack::Entry SymEntry(SymName, SymType, SymValue);
  auto SymNameStr = SymName.GetString();

  if (ToGlobal)
    SymTabStack.InsertGlobalEntry(SymEntry);
  else
    SymTabStack.InsertEntry(SymEntry);
}

bool Parser::IsUserDefined(std::string Name) {
  return UserDefinedTypes.count(Name) > 0 || TypeDefinitions.count(Name);
}

std::vector<Token> Parser::GetUserDefinedTypeMembers(std::string Name) {
  assert(IsUserDefined(Name));

  if (TypeDefinitions.count(Name) > 0)
    Name = TypeDefinitions[Name].GetName();
  return std::get<1>(UserDefinedTypes[Name]);
}

Type Parser::GetUserDefinedType(std::string Name) {
  assert(IsUserDefined(Name));

  if (UserDefinedTypes.count(Name) > 0)
    return std::get<0>(UserDefinedTypes[Name]);
  else
    return TypeDefinitions[Name];
}

bool Parser::IsTypeSpecifier(Token T) {
  switch (T.GetKind()) {
  case Token::Char:
  case Token::Short:
  case Token::Int:
  case Token::Long:
  case Token::Unsigned:
  case Token::Float:
  case Token::Double:
  case Token::Struct:
  case Token::Void:
    return true;
  case Token::Identifier: {
    auto Id = T.GetString();
    if (TypeDefinitions.count(Id) != 0)
      return true;
  }
  default:
    break;
  }
  return false;
}

static bool IsUnaryOperator(Token::TokenKind tk) {
  switch (tk) {
  case Token::And:
  case Token::Astrix:
  case Token::Bang:
  case Token::Minus:
  case Token::MinusMinus:
  case Token::PlusPlus:
  case Token::Sizeof:
  case Token::Tilde:
    return true;

  default:
    return false;
  }
}

bool Parser::IsReturnTypeSpecifier(Token T) {
  return T.GetKind() == Token::Void || IsTypeSpecifier(T);
}

bool IsQualifier(Token::TokenKind tk) {
  switch (tk) {
  case Token::Typedef:
  case Token::Const:
    return true;

  default:
    break;
  }
  return false;
}

bool Parser::IsQualifiedType(Token T) {
  return IsQualifier(T.GetKind()) || IsTypeSpecifier(T);
}

unsigned Parser::ParseQualifiers() {
  unsigned Qualifiers = 0;
  auto CurrTokenKind = GetCurrentTokenKind();

  while (IsQualifier(CurrTokenKind)) {
    Lex(); // eat the qualifier token

    switch (CurrTokenKind) {
    case Token::Typedef:
      Qualifiers |= Type::Typedef;
      break;
    case Token::Const:
      Qualifiers |= Type::Const;
      break;
    default:
      break;
    }
    CurrTokenKind = GetCurrentTokenKind();
  }

  return Qualifiers;
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
  case Token::Short:
    Result.SetTypeVariant(Type::Short);
    break;
  case Token::Int:
    Result.SetTypeVariant(Type::Int);
    break;
  case Token::Long: {
    auto NextTokenKind = lexer.LookAhead(2).GetKind();
    if (NextTokenKind == Token::Long) {
      Lex(); // eat 'long'
      Result.SetTypeVariant(Type::LongLong);
      break;
    }
    Result.SetTypeVariant(Type::Long);
    break;
  }
  case Token::Unsigned: {
    auto NextTokenKind = lexer.LookAhead(2).GetKind();
    if (NextTokenKind == Token::Int || NextTokenKind == Token::Char ||
        NextTokenKind == Token::Short || NextTokenKind == Token::Long)
      Lex(); // eat 'unsigned'
    else {
      // if bare the 'unsigned' is not followed by other type then its an
      // 'unsigned int' by default
      Result.SetTypeVariant(Type::UnsignedInt);
      return Result;
    }

    auto CurrToken = lexer.GetCurrentToken();

    switch (CurrToken.GetKind()) {
    case Token::Char:
      Result.SetTypeVariant(Type::UnsignedChar);
      break;
    case Token::Short:
      Result.SetTypeVariant(Type::UnsignedShort);
      break;
    case Token::Int:
      Result.SetTypeVariant(Type::UnsignedInt);
      break;
    case Token::Long: {
      auto NextTokenKind = lexer.LookAhead(2).GetKind();
      if (NextTokenKind == Token::Long) {
        Lex(); // eat 'long'
        Result.SetTypeVariant(Type::UnsignedLongLong);
        break;
      }
      Result.SetTypeVariant(Type::UnsignedLong);
      break;
    }
    default:
      assert(!"Unreachable");
    }
    break;
  }
  case Token::Float:
    Result.SetTypeVariant(Type::Float);
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
  case Token::Identifier: {
    // assuming we parsing the current token
    // TODO: Change this function expect the Token and not the TokenKind
    assert(GetCurrentTokenKind() == Token::Identifier);
    auto Id = GetCurrentToken().GetString();
    Result = TypeDefinitions[Id];
    break;
  }
  default:
    assert(!"Unknown token kind.");
    break;
  }

  return Result;
}

/// Parse the dimensions of an array declaration
void Parser::ParseArrayDimensions(Type &type) {
  std::vector<unsigned> Dimensions;
  while (lexer.Is(Token::LeftBracet)) {
    Lex(); // consume '['

    // empty dimension like "int arr[];"
    if (lexer.Is(Token::RightBracet)) {
      Lex();
      // using ~0 to signal an unspecified dimension
      Dimensions.push_back(EMPTY_DIMENSION);
    } else {
      // dimension defined by constant case like "int m[5][5]"
      Dimensions.push_back(ParseIntegerConstant());
      Expect(Token::RightBracet);
    }
  }
  if (!Dimensions.empty())
    type.SetDimensions(std::move(Dimensions));
}

/// Helper function to try to figure out the unspecified dimension of the array
/// type @type from its initializer expression @InitExpr
/// example:
///     int a[] = {1, 2, 3}
///
/// since the initializer expression has 3 element, therefore a type is int[3]
void DetermineUnspecifiedDimension(Expression *InitExpr, Type &type) {
  // If there was an initializer expression like "{1, 2, 3}",
  if (auto InitListExpr = dynamic_cast<InitializerListExpression *>(InitExpr);
      InitListExpr != nullptr && type.IsArray() &&
      type.GetDimensions()[0] == EMPTY_DIMENSION) {
    // TODO: only 1 dimensional init list are handled here now, altough C
    // only allows the first dimension to be a unspecified so arr[][] would
    // be invalid anyway
    type.GetDimensions()[0] = InitListExpr->GetExprList().size();
  }
}

// <ExternalDeclaration> ::= <FunctionDeclaration>
//                         | <VariableDeclaration>
//
// First set : {void, int, double}
// Second set : {Identifier}
std::unique_ptr<Node> Parser::ParseExternalDeclaration() {
  std::unique_ptr<TranslationUnit> TU = std::make_unique<TranslationUnit>();
  auto Token = GetCurrentToken();

  while (IsReturnTypeSpecifier(Token) || lexer.Is(Token::Struct) ||
      lexer.Is(Token::Enum) || IsQualifier(Token.GetKind())) {
    auto Qualifiers = ParseQualifiers();
    Token = GetCurrentToken();

    bool IsAlsoStuctVariableDeclaration = false;
    Type BaseType;

    if (lexer.Is(Token::Struct) &&
        (lexer.LookAhead(2).GetKind() == Token::LeftCurly ||
         (lexer.LookAhead(2).GetKind() == Token::Identifier &&
          lexer.LookAhead(3).GetKind() == Token::LeftCurly))) {
      auto SD = ParseStructDeclaration(Qualifiers);
      auto SDPtr = SD.get();

      TU->AddDeclaration(std::move(SD));
      Token = GetCurrentToken();

      if (Token.GetKind() == Token::Identifier ||
          Token.GetKind() == Token::Astrix) {
        IsAlsoStuctVariableDeclaration = true;
        BaseType = std::get<0>(UserDefinedTypes[SDPtr->GetName()]);
      } else {
        Expect(Token::SemiColon);
        Token = GetCurrentToken();
        continue;
      }
    }

    if (lexer.Is(Token::Enum)) {
      TU->AddDeclaration(ParseEnumDeclaration(Qualifiers));
      Token = GetCurrentToken();
      continue;
    }
    
    if (!IsAlsoStuctVariableDeclaration) {
      BaseType = ParseType(Token.GetKind());
      Lex();
    }

    BaseType.SetQualifiers(Qualifiers);
    Type CurrentType = BaseType;

    while (lexer.Is(Token::Astrix)) {
      CurrentType.IncrementPointerLevel();
      Lex(); // Eat the * character
    }

    auto Name = Expect(Token::Identifier);
    bool FailedToFindIdentifier = Name.GetKind() != Token::Identifier;
    auto NameStr = Name.GetString();

    if (Qualifiers & Type::Typedef) {
      TypeDefinitions[NameStr] = CurrentType;
      Expect(Token::SemiColon);
      Token = GetCurrentToken();
      continue;
    }

    // if a function declaration then a left parenthesis '(' expected
    if (lexer.Is(Token::LeftParen) && !FailedToFindIdentifier) {
      CurrentFuncRetType = CurrentType;
      TU->AddDeclaration(ParseFunctionDeclaration(CurrentType, Name));
    } 
    // Variable declaration
    else if (!FailedToFindIdentifier) {
      bool IsFirstIteration = true;

      // since it is possible to have multiple declaration in the same line
      // for example "int *ptr,foo;", therefore it parsed iteratively
      do {
        // these step are already done in the first iteration, only need to do
        // in the following ones
        // TODO: clean up Declaration handling
        if (!IsFirstIteration) {
          while (lexer.Is(Token::Astrix)) {
            CurrentType.IncrementPointerLevel();
            Lex(); // Eat the * character
          }

          Name = Expect(Token::Identifier);
          NameStr = Name.GetString();
        }
        IsFirstIteration = false;

        ParseArrayDimensions(CurrentType);

        // If the variable initialized
        std::unique_ptr<Expression> InitExpr = nullptr;
        if (lexer.Is(Token::Equal)) {
          Lex(); // eat '='
          if (lexer.Is(Token::LeftCurly))
            InitExpr = ParseInitializerListExpression();
          else
            InitExpr = ParseExpression();
        }

        if (CurrentType.IsArray() && !CurrentType.GetDimensions().empty())
          DetermineUnspecifiedDimension(InitExpr.get(), CurrentType);

        InsertToSymTable(Name, CurrentType);

        TU->AddDeclaration(std::make_unique<VariableDeclaration>(
            Name, CurrentType, std::move(InitExpr)));

        // TODO: typedef is not allowed for now, because complex typedefs
        // not supported yet like
        // typedef int int_t, *intptr_t, (&fp)(int, ulong), arr_t[10];
        if (lexer.Is(Token::Comma) && !BaseType.IsTypedef())
          Lex(); // eat ','
        else {
          Expect(Token::SemiColon);
          break;
        }

        CurrentType = BaseType;
      } while (lexer.Is(Token::Astrix) || lexer.Is(Token::Identifier));
    } else if (Name.GetKind() == Token::LeftParen) {
      std::string Msg = "Function pointers are not supported yet";
      ErrorLog.AddNote(Msg, Name);
    }

    Token = GetCurrentToken();
  }

  if (lexer.IsNot(Token::EndOfFile)) {
    std::string Msg;

    if (lexer.Is(Token::Identifier) &&
        (GetCurrentToken().GetString() == "union" ||
         GetCurrentToken().GetString() == "extern" ||
         GetCurrentToken().GetString() == "static" ||
         GetCurrentToken().GetString() == "signed" ||
         GetCurrentToken().GetString() == "_Bool"))
      Msg = "'" + GetCurrentToken().GetString() + "' is not supported yet";
    else
      Msg = "Unexpected token";

    ErrorLog.AddError(Msg, Token);
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

  bool HasVarArg = false;
  auto PL = ParseParameterList(HasVarArg);

  Expect(Token::RightParen);

  auto FuncType = FunctionDeclaration::CreateType(ReturnType, PL);
  if (HasVarArg)
    FuncType.SetVarArg(true);

  InsertToSymTable(Name, FuncType, true);

  ReturnsNumber = 0;
  std::unique_ptr<CompoundStatement> Body = nullptr;
  if (lexer.Is(Token::SemiColon))
    Lex(); // eat ';'
  else
    Body = ParseCompoundStatement();

  // Removing the function's scope since we done with its parsing
  SymTabStack.PopSymTable();

  return std::make_unique<FunctionDeclaration>(FuncType, Name, PL, Body,
                                               ReturnsNumber);
}

// <ParameterList> ::= <ParameterDeclaration>?
//                      {',' <ParameterDeclaration> }* (',' '...')?
std::vector<std::unique_ptr<FunctionParameterDeclaration>>
Parser::ParseParameterList(bool &HasVarArg) {
  std::vector<std::unique_ptr<FunctionParameterDeclaration>> Params;

  if (!IsQualifiedType(GetCurrentToken()) && lexer.Is(Token::RightParen))
    return Params;

  Params.push_back(ParseParameterDeclaration());
  while (lexer.Is(Token::Comma)) {
    Lex(); // consume ','

    // ellipse (...) case
    if (GetCurrentToken().GetKind() == Token::DotDotDot) {
      Lex(); // consume '...'
      HasVarArg = true;
    } else
      Params.push_back(ParseParameterDeclaration());
  }
  return Params;
}

// <ParameterDeclaration> ::= { <TypeSpecifier> '*'* <Identifier>? }?
std::unique_ptr<FunctionParameterDeclaration>
Parser::ParseParameterDeclaration() {
  std::unique_ptr<FunctionParameterDeclaration> FPD =
      std::make_unique<FunctionParameterDeclaration>();

  Type type = ParseTypeSpecifier();
  Lex();

  while (lexer.Is(Token::Astrix)) {
    type.IncrementPointerLevel();
    Lex(); // Eat the * character
  }

  if (lexer.Is(Token::Identifier)) {
    auto Name = Expect(Token::Identifier);
    FPD->SetName(Name);

    // support only empty dimensions for now like "int foo(int a[])"
    if (lexer.Is(Token::LeftBracet)) {
      Lex();
      type.IncrementPointerLevel();
      Expect(Token::RightBracet);
    }

    InsertToSymTable(Name, type);
  }

  FPD->SetType(type);

  return FPD;
}

// TODO: Extend comment
// <TypeSpecifier> ::= int
//                   | double
Type Parser::ParseTypeSpecifier() {
  auto Token = GetCurrentToken();

  unsigned Qualifiers = 0;
  if (IsQualifier(Token.GetKind())) {
    Qualifiers = ParseQualifiers();
    Token = GetCurrentToken();
  }

  if (!IsTypeSpecifier(Token) || (Qualifiers & Type::Typedef)) {
    std::string Msg = "Unexpected token '" + Token.GetString() + "'";
    ErrorLog.AddError(Msg, Token);
    Lex();
    Token = GetCurrentToken();
  }

  auto ParsedType = ParseType(Token.GetKind());
  ParsedType.SetQualifiers(Qualifiers);
  return ParsedType;
}

// <CompoundStatement> ::= '{' <VariableDeclaration>* <Statement>* '}'
std::unique_ptr<CompoundStatement> Parser::ParseCompoundStatement() {
  Expect(Token::LeftCurly);

  std::vector<std::unique_ptr<Statement>> Statements;

  while (
      (IsQualifiedType(GetCurrentToken()) || lexer.IsNot(Token::RightCurly)) &&
      lexer.IsNot(Token::EndOfFile)) {
    if (IsQualifiedType(GetCurrentToken())) {
      auto Declarations = ParseVariableDeclarationList();
      for (auto &Declaration : Declarations)
        Statements.push_back(std::move(Declaration));
    } else
      Statements.push_back(std::move(ParseStatement()));
  }
  Expect(Token::RightCurly);

  return std::make_unique<CompoundStatement>(Statements);
}

// <VariableDeclarationList> ::= <TypeSpecifier> <VariableDeclaration> 
//                             |               {,<VariableDeclaration>} ';'
std::vector<std::unique_ptr<VariableDeclaration>>
Parser::ParseVariableDeclarationList() {
  Type type = ParseTypeSpecifier();
  Lex();

  // using a vector since one line can have multiple declarations like
  // "int a, b;"
  std::vector<std::unique_ptr<VariableDeclaration>> VariableDeclarations;

  while (lexer.IsNot(Token::SemiColon) && lexer.IsNot(Token::EndOfFile)) {
    VariableDeclarations.push_back(ParseVariableDeclaration(type));
    
    if (lexer.Is(Token::Comma))
      Lex(); // consume ','
    else {
      Expect(Token::SemiColon);
      break;
    }
  }

  return VariableDeclarations;
}

// <VariableDeclaration> ::= '*'* <Identifier>
//                           {'[' <IntegerConstant> ]'}* { = <Expression> }?
std::unique_ptr<VariableDeclaration> Parser::ParseVariableDeclaration(Type type) {
  while (lexer.Is(Token::Astrix)) {
    type.IncrementPointerLevel();
    Lex(); // Eat the * character
  }

  Token Name = Expect(Token::Identifier);

  ParseArrayDimensions(type);

  // If the variable initialized
  std::unique_ptr<Expression> InitExpr = nullptr;
  if (lexer.Is(Token::Equal)) {
    Token T = Lex(); // eat '='
    if (lexer.Is(Token::LeftCurly))
      InitExpr = ParseInitializerListExpression();
    else {
      InitExpr = ParseExpression();

      if (InitExpr == nullptr) {
        std::string Msg = "expected expression here";
        ErrorLog.AddError(Msg, T);
        return nullptr;
      }

      // if the variable type not match the size of the initializer expression
      // then also do an implicit cast
      if ((type != InitExpr->GetResultType()) &&
          !Type::OnlySigndnessDifference(
              type.GetTypeVariant(),
              InitExpr->GetResultType().GetTypeVariant())) {

        bool IsImplicitlyCastable = Type::IsImplicitlyCastable(
            InitExpr->GetResultType().GetTypeVariant(), type.GetTypeVariant());

        IsImplicitlyCastable |=
            type.IsPointerType() && InitExpr->GetResultType().IsIntegerType();

        if (!IsImplicitlyCastable) {
          assert(!"Invalid initialization");
        } else {
          InitExpr = std::make_unique<ImplicitCastExpression>(
              std::move(InitExpr), type);
        }
      }
    }
  }

  if (type.IsArray() && !type.GetDimensions().empty())
    DetermineUnspecifiedDimension(InitExpr.get(), type);

  InsertToSymTable(Name, type);

  auto VD = std::make_unique<VariableDeclaration>(Name, type);

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

  Token Name = Expect(Token::Identifier);

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
//                                  '{' <StructDeclarationList>+ '}'
std::unique_ptr<StructDeclaration>
Parser::ParseStructDeclaration(unsigned Qualifiers = 0) {
  Token T = Expect(Token::Struct);

  if (lexer.IsNot(Token::Identifier)) {
    std::string Msg = "unnamed structures are not supported yet";
    ErrorLog.AddNote(Msg, T);
  }

  Token Name = Expect(Token::Identifier);
  auto NameStr = Name.GetString();

  Expect(Token::LeftCurly);

  std::vector<std::unique_ptr<MemberDeclaration>> Members;
  Type type(Type::Struct);
  type.SetName(NameStr);
  type.SetQualifiers(Qualifiers);

  // register the type already even though it is an incomplete type
  // at this time of parsing
  UserDefinedTypes[NameStr] = {type, {}};

  std::vector<Token> StructMemberIdentifiers;
  while (lexer.IsNot(Token::RightCurly)) {
    auto MD = ParseMemberDeclaration();
    type.GetTypeList().push_back(MD->GetType());
    StructMemberIdentifiers.push_back(MD->GetNameToken());
    Members.push_back(std::move(MD));
  }

  Expect(Token::RightCurly);

  if (Qualifiers & Type::Typedef) {
    auto AliasName = Expect(Token::Identifier).GetString();
    TypeDefinitions[AliasName] = type;
  }

  // saving the struct type and name
  UserDefinedTypes[NameStr] = {type, std::move(StructMemberIdentifiers)};

  return std::make_unique<StructDeclaration>(Name, Members, type);
}

// <EnumDeclaration> ::= 'enum' '{' <Identifier> (, <Identifier>)* '}' ';'
std::unique_ptr<EnumDeclaration>
Parser::ParseEnumDeclaration(unsigned Qualifiers) {
  Expect(Token::Enum);
  Expect(Token::LeftCurly);

  EnumDeclaration::EnumList Enumerators;

  int EnumCounter = 0;
  do {
    if (lexer.Is(Token::Comma))
      Lex(); // eat ','

    auto Identifier = Expect(Token::Identifier);
    Enumerators.push_back({Identifier.GetString(), EnumCounter});

    // Insert into the symbol table and for now assign the index of the enum
    // to it, not considering explicit assignments like "enum { A = 10 };"
    InsertToSymTable(Identifier, Type(Type::Int), false,
                     ValueType((unsigned)EnumCounter));
    EnumCounter++;
  } while (lexer.Is(Token::Comma));

  if (lexer.Is(Token::Equal)) {
    Token T = GetCurrentToken();
    std::string Msg = "assigning values to enumerations are not supported yet";
    ErrorLog.AddNote(Msg, T);
  }

  Expect(Token::RightCurly);

  if (Qualifiers & Type::Typedef) {
    auto AliasName = Expect(Token::Identifier).GetString();
    TypeDefinitions[AliasName] = Type(Type::Int);
  }

  Expect(Token::SemiColon);

  return std::make_unique<EnumDeclaration>(Enumerators);
}

unsigned Parser::ParseIntegerConstant() {
  Token T = Expect(Token::Integer);
  auto Str = T.GetString();
  unsigned Result = 0;

  // in case if a hex constant, then its value already parsed, return that
  if (T.GetValue() > 0 ||
      (T.GetString().size() > 2 && T.GetString().at(1) == 'x'))
    return T.GetValue();

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
//               | <DoWhileStatement>
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
  if (lexer.Is(Token::Do))
    return ParseDoWhileStatement();
  if (lexer.Is(Token::For))
    return ParseForStatement();
  if (lexer.Is(Token::LeftCurly))
    return ParseCompoundStatement();
  if (lexer.Is(Token::Break))
    return ParseBreakStatement();
  if (lexer.Is(Token::Continue))
    return ParseContinueStatement();
  if (lexer.Is(Token::Return))
    return ParseReturnStatement();
  return ParseExpressionStatement();
}

// <IfStatement> ::= if '(' <Expression> ')' <Statement> {else <Statement>}?
std::unique_ptr<IfStatement> Parser::ParseIfStatement() {
  std::unique_ptr<IfStatement> IS = std::make_unique<IfStatement>();

  Expect(Token::If);
  Token T = Expect(Token::LeftParen);
  auto Condition = ParseExpression();
  if (Condition == nullptr) {
    std::string Msg = "expected expression here";
    ErrorLog.AddError(Msg, T);
  }
  IS->SetCondition(std::move(Condition));
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
  Token T = Expect(Token::LeftParen);
  auto Condition = ParseExpression();
  if (Condition == nullptr) {
    std::string Msg = "expected expression here";
    ErrorLog.AddError(Msg, T);
  }
  SS->SetCondition(std::move(Condition));
  Expect(Token::RightParen);
  Expect(Token::LeftCurly);

  SwitchStatement::VecOfCasesData CasesData;

  unsigned FoundDefaults = 0;

  while (lexer.Is(Token::Case) || lexer.Is(Token::Default)) {
    const bool IsCase = lexer.Is(Token::Case);
    Token T = Lex(); // eat 'case' or 'default'

    std::unique_ptr<Expression> CaseExpr;

    if (IsCase)
      CaseExpr = ParseExpression();

    if (CaseExpr == nullptr && IsCase) {
      std::string Msg = "expected expression here";
      ErrorLog.AddError(Msg, T);
    }

    Expect(Token::Colon);

    SwitchStatement::VecOfStmts Statements;
    while (lexer.IsNot(Token::RightCurly) && lexer.IsNot(Token::Case) &&
           lexer.IsNot(Token::Default))
      Statements.push_back(std::move(ParseStatement()));

    if (IsCase)
      CasesData.push_back({std::move(CaseExpr), std::move(Statements)});
    else {
      FoundDefaults++;
      // TODO: move to semantic check
      if (FoundDefaults > 1) {
        std::string Msg = "multiple default labels in one switch";
        ErrorLog.AddError(Msg, T);
      } else
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
  Token T = Expect(Token::LeftParen);
  auto Condition = ParseExpression();
  if (Condition == nullptr) {
    std::string Msg = "expected expression here";
    ErrorLog.AddError(Msg, T);
  }
  WS->SetCondition(std::move(Condition));
  Expect(Token::RightParen);
  WS->SetBody(std::move(ParseStatement()));

  return WS;
}

// <DoWhileStatement> ::= do <Statement> while '(' <Expression> ')' ';'
std::unique_ptr<DoWhileStatement> Parser::ParseDoWhileStatement() {
  std::unique_ptr<DoWhileStatement> DWS = std::make_unique<DoWhileStatement>();

  Expect(Token::Do);
  DWS->SetBody(std::move(ParseStatement()));
  Expect(Token::While);
  Token T = Expect(Token::LeftParen);
  auto Condition = ParseExpression();
  if (Condition == nullptr) {
    std::string Msg = "expected expression here";
    ErrorLog.AddError(Msg, T);
  }
  DWS->SetCondition(std::move(Condition));
  Expect(Token::RightParen);
  Expect(Token::SemiColon);

  return DWS;
}

// <ForStatement> ::= for '(' <Expression> ';' <Expression> ';' <Expression> ')'
//                            <Statement>
//                  | for '(' <VariableDeclaration> <Expression> ';'
//                            <Expression> ')' <Statement>
std::unique_ptr<ForStatement> Parser::ParseForStatement() {
  std::unique_ptr<ForStatement> FS = std::make_unique<ForStatement>();

  Expect(Token::For);
  Expect(Token::LeftParen);

  SymTabStack.PushSymTable();

  // Parse variable declaration
  if (IsQualifiedType(GetCurrentToken())) {
    auto Declarations = ParseVariableDeclarationList();
    FS->SetVarDecls(std::move(Declarations));
  } else {
    FS->SetInit(std::move(ParseExpression()));
    Expect(Token::SemiColon);
  }
  FS->SetCondition(std::move(ParseExpression()));
  Expect(Token::SemiColon);

  FS->SetIncrement(std::move(ParseExpression()));
  Expect(Token::RightParen);

  FS->SetBody(std::move(ParseStatement()));
  SymTabStack.PopSymTable();

  return FS;
}

// <ExpressionStatement> ::= <Expression>? ';'
std::unique_ptr<ExpressionStatement> Parser::ParseExpressionStatement() {
  std::unique_ptr<ExpressionStatement> ES =
      std::make_unique<ExpressionStatement>();

  if (lexer.IsNot(Token::SemiColon))
    ES->SetExpression(std::move(ParseExpression()));
  auto T = Expect(Token::SemiColon);

  if (T.GetKind() != Token::SemiColon && ES->GetExpression() == nullptr) {
    std::string Msg =
        "Unexpected token '" + GetCurrentToken().GetString() + "'";
    ErrorLog.AddError(Msg, T);
    Lex();
  }

  return ES;
}

// <BreakStatement> ::= 'break' ';'
std::unique_ptr<BreakStatement> Parser::ParseBreakStatement() {
  Expect(Token::Break);
  Expect(Token::SemiColon);
  return std::make_unique<BreakStatement>();
}

// <ContinueStatement> ::= 'continue' ';'
std::unique_ptr<ContinueStatement> Parser::ParseContinueStatement() {
  Expect(Token::Continue);
  Expect(Token::SemiColon);
  return std::make_unique<ContinueStatement>();
}

// <ReturnStatement> ::= return <Expression>? ';'
std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
  ReturnsNumber++;

  Expect(Token::Return);
  auto Expr = ParseExpression();

  if (Expr == nullptr) {
    Expect(Token::SemiColon);
    return std::make_unique<ReturnStatement>(nullptr);
  }

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
  case Token::MinusGreaterThan:
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
//                       | <PostFixExpression> '->' <Identifier>
//                       | ( TypeName ) '{' <Initializer-List> '}'
std::unique_ptr<Expression> Parser::ParsePostFixExpression() {
  auto CurrentToken = lexer.GetCurrentToken();

  // Struct initializing case
  if (lexer.Is(Token::LeftParen) &&
      ((lexer.LookAhead(2).GetKind() == Token::Identifier &&
        IsUserDefined(lexer.LookAhead(2).GetString())) ||
       (lexer.LookAhead(2).GetKind() == Token::Struct &&
        IsUserDefined(lexer.LookAhead(3).GetString())))) {
    Expect(Token::LeftParen);
    if (lexer.Is(Token::Struct))
      Lex();
    auto TypeName = Expect(Token::Identifier).GetString();
    Expect(Token::RightParen);

    Expect(Token::LeftCurly);

    std::vector<Token> InitializedMemberList;
    StructInitExpression::ExprPtrList InitList;
    while (lexer.Is(Token::Dot) || lexer.Is(Token::Identifier)) {
      Token Member;
      if (lexer.Is(Token::Dot)) {
        Lex(); // eat '.'
        Member = Expect(Token::Identifier);
        Expect(Token::Equal);
      }

      InitializedMemberList.push_back(Member);
      InitList.push_back(ParseExpression());

      if (!lexer.Is(Token::Comma))
        break;

      Lex(); // eat ','
    }
    Expect(Token::RightCurly);

    // construct a list of the orders how the fields are initialized
    // ex.: "struct P { int x, y; };"
    //      ...
    //      struct P Obj = (struct P) { .y = 2, .x = 1 }
    //
    //  in the above case the InitOrder would look like: {1, 0}, so the first
    //  init expression actually initializing the 2nd (index 1) struct member
    auto MemberNames = GetUserDefinedTypeMembers(TypeName);
    std::vector<unsigned> InitOrder;

    for (auto &Member : InitializedMemberList) {
      unsigned Order = 0;
      bool Found = false;

      for (auto &TypeMemberName : MemberNames) {
        if (TypeMemberName == Member) {
          InitOrder.push_back(Order);
          Found = true;
          break;
        }
        Order++;
      }

      // TODO: move this to semantics
      if (!Found) {
        auto StructType = GetUserDefinedType(TypeName);
        std::string Msg = "'" + StructType.ToString() +
                          "' has no member named '" + Member.GetString() + "'";
        ErrorLog.AddError(Msg, Member);
      }
    }

    auto ResTy = GetUserDefinedType(TypeName);
    return std::make_unique<StructInitExpression>(
        ResTy, std::move(InitList),
        std::move(InitOrder));
  }

  auto Expr = ParsePrimaryExpression();
  if (!Expr)
    return nullptr;

  while (IsPostfixOperator(lexer.GetCurrentToken())) {
    if (lexer.Is(Token::PlusPlus) || lexer.Is(Token::MinusMinus)) {
      auto Operation = lexer.GetCurrentToken();
      Lex(); // eat the token
      Expr->SetLValueness(true);
      Expr = std::make_unique<UnaryExpression>(Operation, std::move(Expr), true);
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
    else if (lexer.Is(Token::Dot) || lexer.Is(Token::MinusGreaterThan)) {
      const bool IsArrow = lexer.Is(Token::MinusGreaterThan);
      Lex(); // eat the token
      auto MemberId = Expect(Token::Identifier);
      auto MemberIdStr = MemberId.GetString();

      // find the type of the member
      auto StructDataTuple = UserDefinedTypes[Expr->GetResultType().GetName()];
      auto StructType = std::get<0>(StructDataTuple);
      auto StructMemberNames = std::get<1>(StructDataTuple);

      size_t MemberIndex = -1;
      for (size_t i = 0; i < StructMemberNames.size(); i++)
        if (StructMemberNames[i] == MemberId) {
          MemberIndex = i;
          break;
        }

      Expr = std::make_unique<StructMemberReference>(std::move(Expr), MemberId,
                                                     MemberIndex, IsArrow);
      if (Expr->GetResultType().IsStruct() || Expr->GetResultType().IsArray())
        Expr->SetLValueness(true);
    }
  }

  return Expr;
}

std::unique_ptr<Expression> Parser::ParseUnaryExpression() {
  auto UnaryOperation = GetCurrentToken();

  // cast expression case
  if (GetCurrentToken().GetKind() == Token::LeftParen &&
      IsTypeSpecifier(lexer.LookAhead(2)) &&
      // and it is not a struct initialization like "(StructType) { ..."
      !((lexer.LookAhead(2).GetKind() == Token::Identifier &&
        lexer.LookAhead(4).GetKind() == Token::LeftCurly) ||
      (lexer.LookAhead(2).GetKind() == Token::Struct &&
       lexer.LookAhead(5).GetKind() == Token::LeftCurly))) {
    Lex(); // eat the '('

    auto type = ParseType(GetCurrentToken().GetKind());
    Lex();

    while (lexer.Is(Token::Astrix)) {
      type.IncrementPointerLevel();
      Lex(); // Eat the * character
    }

    Expect(Token::RightParen);

    auto ExprToCast = ParseExpression();
    return std::make_unique<ImplicitCastExpression>(std::move(ExprToCast), type,
                                                    true);
  }

  if (!IsUnaryOperator(UnaryOperation.GetKind()))
    return ParsePostFixExpression();

  Lex(); // eat the unary operation char

  std::unique_ptr<Expression> Expr;
  bool hasSizeofParenthesis = false;

  // 'sizeof' handling
  if (UnaryOperation.GetKind() == Token::Sizeof) {
    if (lexer.GetCurrentToken().GetKind() == Token::LeftParen) {
      Lex();
      hasSizeofParenthesis = true;
    }

    if (IsTypeSpecifier(lexer.GetCurrentToken())) {
      auto type = ParseType(lexer.GetCurrentToken().GetKind());
      Lex();

      while (lexer.Is(Token::Astrix)) {
        type.IncrementPointerLevel();
        Lex(); // Eat the * character
      }

      if (hasSizeofParenthesis)
        Expect(Token::RightParen);

      auto UE = std::make_unique<UnaryExpression>(UnaryOperation, nullptr);
      UE->SetSizeOfType(type);
      return UE;
    }
  }

  if (IsUnaryOperator((UnaryOperation.GetKind()))) {
    auto Expr = ParseUnaryExpression();

    if (hasSizeofParenthesis)
      Expect(Token::RightParen);

    if (UnaryOperation.GetKind() == Token::PlusPlus ||
        UnaryOperation.GetKind() == Token::MinusMinus)
      Expr->SetLValueness(true);
    return std::make_unique<UnaryExpression>(UnaryOperation, std::move(Expr));
  }

  // TODO: Add semantic check that only pointer types are dereferenced
  Expr = ParsePostFixExpression();

  if (hasSizeofParenthesis)
    Expect(Token::RightParen);

  if (UnaryOperation.GetKind() == Token::PlusPlus ||
      UnaryOperation.GetKind() == Token::MinusMinus)
    Expr->SetLValueness(true);
  return std::make_unique<UnaryExpression>(UnaryOperation, std::move(Expr));
}

static int GetBinOpPrecedence(Token::TokenKind TK) {
  switch (TK) {
  case Token::Equal:
  case Token::PlusEqual:
  case Token::MinusEqual:
  case Token::AstrixEqual:
  case Token::ForwardSlashEqual:
  case Token::PercentEqual:
  case Token::AndEqual:
  case Token::OrEqual:
  case Token::CaretEqual:
  case Token::LessThanLessThanEqual:
  case Token::GreaterThanGreaterThanEqual:
    return 10;
  case Token::DoubleOr:
    return 20;
  case Token::DoubleAnd:
    return 30;
  case Token::Or:
    return 40;
  case Token::Caret:
    return 50;
  case Token::And:
    return 60;
  case Token::DoubleEqual:
  case Token::BangEqual:
  case Token::GreaterEqual:
  case Token::LessEqual:
    return 70;
  case Token::LessThan:
  case Token::GreaterThan:
    return 80;
  case Token::LessThanLessThan:
  case Token::GreaterThanGreaterThan:
    return 90;
  case Token::Plus:
  case Token::Minus:
    return 100;
  case Token::Astrix:
  case Token::ForwardSlash:
  case Token::Percent:
    return 110;
  default:
    return -1;
  }
}

std::unique_ptr<Expression> Parser::ParseBinaryExpression() {
  auto LeftExpression = ParseUnaryExpression();
  if(!LeftExpression)
    return nullptr;

  // TODO: see other call sites...
  if (lexer.Is(Token::QuestionMark))
    LeftExpression = ParseTernaryExpression(std::move(LeftExpression));

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

    bool IsArithmetic = false;
    switch (BinaryOperator.GetKind()) {
    case Token::Plus:
    case Token::Minus:
    case Token::Astrix:
    case Token::ForwardSlash:
      IsArithmetic = true;
      break;
    default:
      break;
    }

    bool IsAssignment = false;
    switch (BinaryOperator.GetKind()) {
    case Token::Equal:
    case Token::PlusEqual:
    case Token::MinusEqual:
    case Token::AstrixEqual:
    case Token::ForwardSlashEqual:
    case Token::PercentEqual:
    case Token::AndEqual:
    case Token::OrEqual:
    case Token::CaretEqual:
    case Token::LessThanLessThanEqual:
    case Token::GreaterThanGreaterThanEqual:
      IsAssignment = true;
      break;
    default:
      break;
    }

    if (IsArithmetic &&
        Type::IsSmallerThanInt(LeftExpression->GetResultType().GetTypeVariant())) {
      LeftExpression = std::make_unique<ImplicitCastExpression>(
          std::move(LeftExpression), Type(Type::Int));
    }

    if (IsArithmetic &&
        Type::IsSmallerThanInt(RightExpression->GetResultType().GetTypeVariant())) {
      RightExpression = std::make_unique<ImplicitCastExpression>(
          std::move(RightExpression), Type(Type::Int));
    }

    //  If it is an assignment and the left hand side is an LValue.
    // TODO: Should be solved in a better way. Seems like LLVM using
    // ImplicitCast for this purpose as well. Should investigate that solution.
    if (IsAssignment)
      if (dynamic_cast<ArrayExpression *>(LeftExpression.get()) ||
          dynamic_cast<ReferenceExpression *>(LeftExpression.get()) ||
          dynamic_cast<StructMemberReference *>(LeftExpression.get()) ||
          (dynamic_cast<UnaryExpression *>(LeftExpression.get()) &&
           dynamic_cast<UnaryExpression *>(LeftExpression.get())
                   ->GetOperationKind() == UnaryExpression::DEREF))
        LeftExpression->SetLValueness(true);

    // convert left expression to RValue if the operation is not assignment
    if (!IsAssignment)
      LeftExpression->SetLValueness(false);
    
    // convert right expression to RValue
    RightExpression->SetLValueness(false);

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
    auto LeftType = LeftExpression->GetResultType();
    auto RightType = RightExpression->GetResultType();

    // Having different types.
    if (LeftType != RightType) {
      // if an assignment, then try to cast the right-hand side to type of the
      // left-hand side if it is allowed
      if (IsAssignment) {
        if (Type::IsImplicitlyCastable(RightType, LeftType))
          RightExpression = std::make_unique<ImplicitCastExpression>(
              std::move(RightExpression), LeftType);
      }
      // Otherwise cast the one with lower conversion rank to the higher one
      else if (Type::IsImplicitlyCastable(RightType, LeftType) ||
               Type::IsImplicitlyCastable(LeftType, RightType)) {
        auto DesiredType = Type::GetStrongestType(LeftType.GetTypeVariant(),
                                                  RightType.GetTypeVariant())
                               .GetTypeVariant();

        const bool IsLeftPtr = LeftType.IsPointerType();
        const bool LeftNeedConv = LeftType != DesiredType && !IsLeftPtr;

        // If left-hand side needs the conversion
        if (LeftNeedConv)
          LeftExpression = std::make_unique<ImplicitCastExpression>(
              std::move(LeftExpression), RightType);
        else // if the right one
          RightExpression = std::make_unique<ImplicitCastExpression>(
              std::move(RightExpression), LeftType);
      }
    }

    // TODO: This will only work here if the ternary condition was in
    //  parenthesis, which for the time being is sufficient. Make it work as it
    //  should.
    if (lexer.Is(Token::QuestionMark))
      RightExpression = ParseTernaryExpression(std::move(RightExpression));

    LeftExpression = std::make_unique<BinaryExpression>(
        std::move(LeftExpression), BinaryOperator, std::move(RightExpression));
  }
}

// <TernaryExpression> ::= <Expression> '?' <Expression> ':' <Expression>
std::unique_ptr<Expression>
Parser::ParseTernaryExpression(std::unique_ptr<Expression> Condition) {
  Expect(Token::QuestionMark);
  auto TrueExpr = ParseExpression();
  Expect(Token::Colon);
  auto FalseExpr = ParseExpression();

  return std::make_unique<TernaryExpression>(Condition, TrueExpr, FalseExpr);
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
  } else if (lexer.Is(Token::Real) || lexer.Is(Token::Integer) ||
             lexer.Is(Token::CharacterLiteral) ||
             lexer.Is(Token::StringLiteral)) {
    return ParseConstantExpression();
  } else {
    return nullptr;
  }
}

// <ConstantExpression> ::= -?[1-9][0-9]*
//                        | -?[0-9]+.[0-9]+
//                        | -?'\'' \?. '\''
std::unique_ptr<Expression> Parser::ParseConstantExpression() {
  // Handle enumerator constant cases
  bool IsNegative = false;
  if (lexer.Is(Token::Minus)) {
    Lex(); // eat '-'
    IsNegative = true;
  }

  // character constant
  if (lexer.Is(Token::CharacterLiteral)) {
    auto CharToken = Expect(Token::CharacterLiteral);

    auto IntLit =
        std::make_unique<IntegerLiteralExpression>(CharToken.GetValue());
    if (IsNegative)
      IntLit->SetValue(-IntLit->GetSIntValue());

    return IntLit;
  } else if (lexer.Is(Token::StringLiteral)) {
    // TODO: we may already parsed a minus which is not valid for string lit
    // move this whole case maybe at the start of the function 
    auto StringToken = Expect(Token::StringLiteral);

    assert(StringToken.GetString().length() >= 2);
    return std::make_unique<StringLiteralExpression>(
        // removing the quotes (") with substr
        StringToken.GetString().substr(1,
                                       StringToken.GetString().length() - 2));
  } else if (lexer.Is(Token::Identifier)) {
    auto Id = Expect(Token::Identifier);
    auto IdStr = Id.GetString();

    if (auto SymEntry = SymTabStack.Contains(IdStr)) {
      if (auto Val = std::get<2>(SymEntry.value()); !Val.IsEmpty()) {
        auto Enum = std::make_unique<IntegerLiteralExpression>(Val.GetIntVal());
        if (IsNegative)
          Enum->SetValue(-Enum->GetSIntValue());
        return Enum;
      } else
        assert(!"Not an enumerator constant");
    } else
      assert(!"Not an enumerator constant");
  } else if (lexer.Is(Token::Integer)) {
    auto IntLit = std::make_unique<IntegerLiteralExpression>(ParseIntegerConstant());
    if (IsNegative)
      IntLit->SetValue(-IntLit->GetSIntValue());
    // TODO: currently 1 ull would be valid since the lexer will ignore the
    // white spaces, make such input invalid
    if (lexer.Is(Token::Identifier)) {
      auto Str = GetCurrentToken().GetString();
      if (Str == "u") {
        Lex();
        IntLit->SetType(Type::UnsignedInt);
      } else if (Str == "l") {
        Lex();
        IntLit->SetType(Type::Long);
      } else if (Str == "ul") {
        Lex();
        IntLit->SetType(Type::UnsignedLong);
      } else if (Str == "ll") {
        Lex();
        IntLit->SetType(Type::LongLong);
      } else if (Str == "ull") {
        Lex();
        IntLit->SetType(Type::UnsignedLongLong);
      }
    }
    return IntLit;
  } else {
    auto FPLit = std::make_unique<FloatLiteralExpression>(ParseRealConstant());
    if (IsNegative)
      FPLit->SetValue(-FPLit->GetValue());
    return FPLit;
  }
}

std::unique_ptr<Expression> Parser::ParseCallExpression(Token Id) {
  assert(Id.GetKind() == Token::Identifier && "Identifier expected");
  Lex(); // eat the '('

  Type FuncType = Type::Int; // default return type is int

  if (auto SymEntry = SymTabStack.Contains(Id.GetString()))
    FuncType = std::get<1>(SymEntry.value());

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
    for (size_t i = 0; i < std::min(FuncArgNum, CallArgs.size()); i++) {
      auto CallArgType = CallArgs[i]->GetResultType();

      // If the ith argument type is not matching the expected one
      if (CallArgType != FuncArgTypes[i]) {
        // Cast if allowed
        if (Type::IsImplicitlyCastable(CallArgType, FuncArgTypes[i]))
          CallArgs[i] = std::make_unique<ImplicitCastExpression>(
              std::move(CallArgs[i]), FuncArgTypes[i]);
      }
    }
  }

  Expect(Token::RightParen);

  return std::make_unique<CallExpression>(Id, CallArgs, FuncType);
}

std::unique_ptr<Expression>
Parser::ParseArrayExpression(std::unique_ptr<Expression> Base) {
  Lex();
  auto IndexExpr = ParseExpression();
  Expect(Token::RightBracet);

  Type type = Base->GetResultType();

  /// Remove the first dimensions from the actual type. Example:
  /// ActualType is 'int arr[5][10]' and our reference is 'arr[0]'
  /// then the result type of 'arr[0]' is 'int[10]'.
  if (!type.IsPointerType() && type.IsArray()) {
    type.GetDimensions().erase(type.GetDimensions().begin());
    // if the result is now a scalar, then change the type to Simple (scalar)
    if (type.GetDimensions().empty())
      type.SetTypeKind(Type::Simple);
  } else if (type.IsPointerType())
    type.DecrementPointerLevel();

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

  return RE;
}

// <InitializerListExpression> ::= '{' {<ConstantExpression> |
//                                      <InitializerListExpression>}
//                                     {',' {<ConstantExpression> |
//                                      <InitializerListExpression>} }* '}'
std::unique_ptr<Expression> Parser::ParseInitializerListExpression() {
  Expect(Token::LeftCurly);
  std::unique_ptr<Expression> E;
  if (lexer.Is(Token::LeftCurly))
    E = ParseInitializerListExpression();
  else
    E = ParseConstantExpression();
  assert(E && "Cannot be null");

  std::vector<std::unique_ptr<Expression>> ExprList;
  ExprList.push_back(std::move(E));

  while (lexer.Is(Token::Comma)) {
    Lex(); // eat ','
    if (lexer.Is(Token::LeftCurly))
      ExprList.push_back(std::move(ParseInitializerListExpression()));
    else
      ExprList.push_back(std::move(ParseConstantExpression()));
  }

  Expect(Token::RightCurly);

  return std::make_unique<InitializerListExpression>(std::move(ExprList));
}
