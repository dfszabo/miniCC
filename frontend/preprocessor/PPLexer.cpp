#include "PPLexer.hpp"
#include <cassert>
#include <cctype>

std::unordered_map<std::string, PPToken::PPTokenKind> PPLexer::Keywords =
    std::unordered_map<std::string, PPToken::PPTokenKind>{
        {"define", PPToken::Define},
    };

PPLexer::PPLexer(std::string &s) {
  Source = s;
  PPTokenBuffer = std::vector<PPToken>();
  LineIndex = 0;

  LookAhead(1);
}

void PPLexer::ConsumeCurrentPPToken() {
  assert(PPTokenBuffer.size() > 0 && "PPTokenBuffer is empty.");
  PPTokenBuffer.erase(PPTokenBuffer.begin());
}

int PPLexer::GetNextChar() {
  if (LineIndex >= Source.size())
    return EOF;
  return Source[LineIndex];
}

int PPLexer::GetNextNthCharOnSameLine(unsigned n) {
  if (LineIndex >= Source.size())
    return EOF;
  return Source[LineIndex + n];
}

void PPLexer::EatNextChar() {
  LineIndex++;
}

std::optional<PPToken> PPLexer::LexIdentifier() {
  unsigned StartLineIndex = LineIndex;
  unsigned Length = 0;

  // Cannot start with a digit
  if (isdigit(GetNextChar()))
    return std::nullopt;

  while (isalnum(GetNextChar()) || GetNextChar() == '_') {
    Length++;
    EatNextChar();
  }

  if (Length == 0)
    return std::nullopt;

  std::string_view StringValue{&Source[StartLineIndex], Length};
  return PPToken(PPToken::Identifier, StringValue);
}

std::optional<PPToken> PPLexer::LexKeyword() {
  std::size_t WordEnd =
      Source.substr(LineIndex).find_first_of("\t\n\v\f\r;: ");

  auto Word = Source.substr(LineIndex, WordEnd);

  if (!Keywords.count(Word))
    return std::nullopt;

  unsigned StartLineIndex = LineIndex;

  for (int i = Word.length(); i > 0; i--)
    EatNextChar();

  std::string_view StringValue{&Source[StartLineIndex], Word.length()};
  return PPToken(PPLexer::Keywords[Word], StringValue);
}

std::optional<PPToken> PPLexer::LexSymbol() {
  auto PPTokenKind = PPToken::Invalid;

  switch (GetNextChar()) {
  case ',':
    PPTokenKind = PPToken::Colon;
    break;
  case '#':
    PPTokenKind = PPToken::Hashtag;
    break;
  case '(':
    PPTokenKind = PPToken::LeftParen;
    break;
  case ')':
    PPTokenKind = PPToken::RightParen;
    break;
  default:
    return std::nullopt;
    break;
  }

  std::string_view StringValue{&Source[LineIndex], 1};
  auto Result = PPToken(PPTokenKind, StringValue);

  EatNextChar();

  return Result;
}

PPToken PPLexer::LookAhead(unsigned n) {
  // fill in the PPTokenBuffer to have at least n element
  for (size_t i = PPTokenBuffer.size(); i < n; i++)
    PPTokenBuffer.push_back(Lex(true));

  return PPTokenBuffer[n - 1];
}

bool PPLexer::Is(PPToken::PPTokenKind tk) {
  // fill in the buffer with one token if it is empty
  if (PPTokenBuffer.size() == 0)
    LookAhead(1);

  return GetCurrentPPToken().GetKind() == tk;
}

bool PPLexer::IsNot(PPToken::PPTokenKind tk) { return !Is(tk); }

PPToken PPLexer::Lex(bool LookAhead) {
  // if the PPTokenBuffer not empty then return the PPToken from there
  // and remove it from the stack
  if (PPTokenBuffer.size() > 0 && !LookAhead) {
    auto CurrentPPToken = GetCurrentPPToken();
    ConsumeCurrentPPToken();
    return CurrentPPToken;
  }

  int CurrentCharacter = GetNextChar();
  std::string WhiteSpaceChars("\t\n\v\f\r ");

  // consume white space characters
  while (WhiteSpaceChars.find(CurrentCharacter) != std::string::npos) {
    EatNextChar();
    CurrentCharacter = GetNextChar();
  }

  if (CurrentCharacter == EOF)
    return PPToken(PPToken::EndOfFile);

  auto Result = LexKeyword();

  if (!Result)
    Result = LexSymbol();
  if (!Result)
    Result = LexIdentifier();

  if (Result)
    return Result.value();

  return PPToken(PPToken::Invalid);
}
