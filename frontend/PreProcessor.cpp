#include "PreProcessor.hpp"

#include <cassert>

std::string WhiteSpaceChars("\t ");

void PreProcessor::ParseDirective(std::string &Line) {
  std::string Directive = Line.c_str() + 1;
  auto pos = Directive.find_first_not_of("abcdefghijklmnopqrstuvwxyz ");
  if (pos != std::string::npos)
    Directive = Directive.substr(0, pos - 1);

  // TODO: make a lexer for the preprocessor too
  if (Directive == "define") {
    size_t CurrentLinePos = 1 /* # */ + sizeof("define");
    // find next non white space character position
    while (CurrentLinePos < Line.length() &&
           WhiteSpaceChars.find(Line[CurrentLinePos]) != std::string::npos)
      CurrentLinePos++;

    assert(CurrentLinePos < Line.length() && "Empty define directive");

    size_t StartPosOfID = CurrentLinePos;
    while (isalpha(Line[CurrentLinePos]) || Line[CurrentLinePos] == '_')
      CurrentLinePos++;

    size_t EndPosOfID = CurrentLinePos;

    // find next non white space character position
    while (CurrentLinePos < Line.length() &&
           WhiteSpaceChars.find(Line[CurrentLinePos]) != std::string::npos)
      CurrentLinePos++;

    std::string MacroID = Line.substr(StartPosOfID, EndPosOfID - StartPosOfID);
    if (CurrentLinePos >= Line.length()) {
      DefinedMacros[MacroID] = "";
    } else {
      DefinedMacros[MacroID] = Line.substr(CurrentLinePos,
                                           Line.length() - CurrentLinePos);
    }
  }
}

void PreProcessor::SubstituteMacros(std::string &Line) {
  for (auto &[MacroID, MacroBody] : DefinedMacros)
    if (Line.find(MacroID) != std::string::npos)
      Line.replace(Line.find(MacroID), MacroID.length(), MacroBody);
}

void PreProcessor::Run() {
  for (size_t LineIdx = 0; LineIdx < Source.size(); LineIdx++) {
    auto &Line = Source[LineIdx];

    if (!Line.empty() && Line[0] == '#') {
      ParseDirective(Line);
      // delete current line, assuming the directive only used one line
      Source.erase(Source.begin() + LineIdx);
      LineIdx--; // since the erase we have to check again the same LineIdx
    }
    else if (!DefinedMacros.empty())
      SubstituteMacros(Line);
  }
}

