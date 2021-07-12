#ifndef PPLEXER_H
#define PPLEXER_H

#include "PPToken.hpp"
#include <cassert>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class PPLexer {
public:
  void ConsumeCurrentPPToken();
  int GetNextChar();
  int GetNextNthCharOnSameLine(unsigned n);

  // Update LineIndex to make them pointing to the next input character
  void EatNextChar();

  std::optional<PPToken> LexIdentifier();
  std::optional<PPToken> LexKeyword();
  std::optional<PPToken> LexSymbol();
  PPToken LookAhead(unsigned n);
  PPToken GetCurrentPPToken() { return LookAhead(1); }
  bool Is(PPToken::PPTokenKind tk);
  bool IsNot(PPToken::PPTokenKind tk);

  std::string &GetSource() { return Source; }
  std::string GetRemainingText() {
    assert(LineIndex < Source.size());
    return Source.substr(LineIndex);
  }

  unsigned GetLineNum() const { return LineIndex + 1; }

  PPToken Lex(bool LookAhead = false);

  PPLexer(std::string &s);

private:
  static std::unordered_map<std::string, PPToken::PPTokenKind> Keywords;
  std::string Source;
  std::vector<PPToken> PPTokenBuffer;
  unsigned LineIndex = 0;
};

#endif