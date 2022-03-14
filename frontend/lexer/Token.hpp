#ifndef TOKEN_H
#define TOKEN_H

#include <cassert>
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
    CharacterLiteral,
    StringLiteral,

    // Operators
    Dot,
    Comma,
    Plus,
    Minus,
    Astrix,
    ForwardSlash,
    Percent,
    Equal,
    LessThan,
    GreaterThan,
    Bang,
    And,
    Or,
    Caret,
    Tilde,
    QuestionMark,
    Sizeof,

    // Multichar operators
    PlusPlus,
    MinusMinus,
    PlusEqual,
    MinusEqual,
    AstrixEqual,
    ForwardSlashEqual,
    PercentEqual,
    LessThanLessThanEqual,
    GreaterThanGreaterThanEqual,
    AndEqual,
    OrEqual,
    CaretEqual,
    BangEqual,
    GreaterEqual,
    LessEqual,
    DoubleAnd,
    DoubleOr,
    DoubleEqual,
    MinusGreaterThan,
    LessThanLessThan,
    GreaterThanGreaterThan,

    // Symbols
    Colon,
    SemiColon,
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    LeftCurly,
    RightCurly,
    BackSlash,
    DotDotDot,

    // Comment
    DoubleForwardSlash,
    ForwardSlashAstrix,
    AstrixForwardSlash,

    // Keywords
    For,
    While,
    Do,
    If,
    Switch,
    Case,
    Default,
    Break,
    Continue,
    Else,
    Return,
    Const,
    Char,
    Short,
    Int,
    Long,
    Float,
    Double,
    Unsigned,
    Void,
    Struct,
    Enum,
    Typedef,

    // C11
    Bool,
    Alignas,
    Alignof,
    Atomic,
    Complex,
    Generic,
    Imaginary,
    Noreturn,
    StaticAssert,
    ThreadLocal,
  };

  Token() : Kind(Invalid) {}

  explicit Token(TokenKind tk) : Kind(tk) {}

  Token(TokenKind tk, std::string_view sv, unsigned l, unsigned c)
      : Kind(tk), StringValue(sv), LineNumber(l), ColumnNumber(c) {}

  Token(TokenKind tk, std::string_view sv, unsigned l, unsigned c, unsigned v)
      : Kind(tk), StringValue(sv), LineNumber(l), ColumnNumber(c), Value(v) {}

  [[nodiscard]] std::string GetString() const {
    return std::string(StringValue);
  }
  [[nodiscard]] TokenKind GetKind() const { return Kind; }

  [[nodiscard]] unsigned GetLineNum() const { return LineNumber; }
  [[nodiscard]] unsigned GetColNum() const { return ColumnNumber; }
  [[nodiscard]] unsigned GetValue() const { return Value; }

  [[nodiscard]] std::string ToString() const;

  static std::string ToString(TokenKind tk);

  bool operator==(const Token &RHS) {
    return GetString() == RHS.GetString() && Kind == RHS.Kind;
  }

private:
  TokenKind Kind;
  std::string_view StringValue;
  unsigned LineNumber{};
  unsigned ColumnNumber{};
  unsigned Value = 0;
};

#endif