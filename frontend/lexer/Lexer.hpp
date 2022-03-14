#ifndef LEXER_H
#define LEXER_H

#include "Token.hpp"
#include <cassert>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Lexer {
public:
  void ConsumeCurrentToken();
  int GetNextChar();
  int GetNextNthCharOnSameLine(unsigned n);

  // Update LineIndex and ColumnIndex to make them point to the next
  // input character
  void EatNextChar();

  // For matching an integer or real number
  std::optional<Token> LexNumber();
  std::optional<Token> LexIdentifier();
  std::optional<Token> LexKeyword();
  std::optional<Token> LexCharLiteral();
  std::optional<Token> LexStringLiteral();
  std::optional<Token> LexSymbol();
  Token LookAhead(unsigned n);
  Token GetCurrentToken() { return LookAhead(1); }
  bool Is(Token::TokenKind tk);
  bool IsNot(Token::TokenKind tk);

  Token Lex(bool LookAhead = false);

  explicit Lexer(std::vector<std::string> &s);

private:
  static std::unordered_map<std::string, Token::TokenKind> Keywords;
  std::vector<std::string> Source;
  std::vector<Token> TokenBuffer;
  unsigned LineIndex;
  unsigned ColumnIndex;
};

#endif