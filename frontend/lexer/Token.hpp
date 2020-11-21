#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <unordered_map>

class Token {
public:
  enum TokenKind {
    EndOfFile,
    Invalid,

    Identifier,

    // Numbers
    Integer,
    Real,

    // Operators
    Colon,
    Plus,
    Minus,
    Astrix,
    ForwardSlash,
    Percent,
    Equal,
    LessThan,
    GreaterThan,
    Bang,
    BangEqual,
    And,
    DoubleAnd,

    // Multichar operators
    DoubleEqual,

    // Symbols
    SemiColon,
    LeftParen,
    RightParen,
    LeftBracet,
    RightBracet,
    LeftCurly,
    RightCurly,

    // Keywords
    For,
    While,
    If,
    Else,
    Return,
    Int,
    Double,
    Void
  };

  Token() : Kind(Invalid) {}

  Token(TokenKind tk) : Kind(tk) {}

  Token(TokenKind tk, std::string_view sv, unsigned l, unsigned c)
      : Kind(tk), StringValue(sv), LineNumber(l), ColumnNumber(c) {}

  std::string GetString() { return std::string(StringValue); }
  TokenKind GetKind() { return Kind; }

  std::string ToString() {
    std::string Result("");
    Result += "\"" + std::string(StringValue) + "\", ";
    Result += "Line: " + std::to_string(LineNumber + 1) + ", ";
    Result += "Col: " + std::to_string(ColumnNumber + 1);
    return Result;
  }

private:
  TokenKind Kind;
  std::string_view StringValue;
  unsigned LineNumber;
  unsigned ColumnNumber;
};

#endif