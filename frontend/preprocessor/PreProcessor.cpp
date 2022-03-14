#include "PreProcessor.hpp"
#include "PPLexer.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>

static bool getFileContent(const std::string &fileName,
                           std::vector<std::string> &vecOfStrs) {
  // Open the File
  std::ifstream in(fileName.c_str());
  // Check if object is valid
  if (!in)
    return false;

  std::string str;
  // Read the next line from File until it reaches the end.
  while (std::getline(in, str))
    // Line contains string of length > 0 then save it in vector
    vecOfStrs.push_back(str);

  // Close The File
  in.close();
  return true;
}

void PreProcessor::ParseDirective(std::string &Line, size_t LineIdx) {
  PPLexer lexer(Line);
  lexer.Lex(); // eat '#'

  auto Directive = lexer.Lex();
  assert(Directive.IsKeyword() && "Must be a keyword at this point");

  if (Directive.GetKind() == PPToken::Define) {
    auto DefinedID = lexer.Lex();
    assert(DefinedID.GetKind() != PPToken::EndOfFile &&
           DefinedID.GetKind() == PPToken::Identifier);

    // must be called here otherwise the token lookaheads mess up the lineindex
    // TODO: solve this problem, maybe with giving tokens the linenumber
    auto RemainingText = lexer.GetRemainingText();

    // The lexer ignores spaces so only looking at tokens currently impossible
    // to distinguish between something like "assert (" and "assert(". But this
    // needed to know its a function like macro or a simple find and replace
    // one.
    // TODO: Solve this issue in a better way
    bool NoSpaceBetweenIDAndParen =
        lexer.GetSource()[lexer.GetLineIndex()] == '(';

    if (lexer.Is(PPToken::EndOfFile))
      DefinedMacros[DefinedID.GetString()] = {"", 0};
    else if (lexer.Is(PPToken::LeftParen) && NoSpaceBetweenIDAndParen) {
      lexer.Lex(); // eat '('
      std::vector<std::string> Params;

      do {
        auto Param = lexer.Lex();
        assert(Param.GetKind() == PPToken::Identifier);
        Params.push_back(Param.GetString());

        if (lexer.IsNot(PPToken::Colon))
          break;
        lexer.Lex(); // eat ','
      } while (true);

      assert(lexer.Is(PPToken::RightParen));
      lexer.Lex(); // eat ')'

      auto Body = lexer.GetRemainingText();
      for (size_t i = 0; i < Params.size(); i++) {
        // replacing the parameters with their index e.g.: with the below macro
        //    #define MAX(A,B) (((A) > (B)) ? (A) : (B))
        // the Body is "(((A) > (B)) ? (A) : (B))"
        // and it became "((($0) > ($1)) ? ($0) : ($1))"
        // this will make the substitution easier later
        while (Body.find(Params[i]) != std::string::npos)
          Body.replace(Body.find(Params[i]), Params[i].length(),
                       "$" + std::to_string(i));
      }

      DefinedMacros[DefinedID.GetString()] = {Body, Params.size()};
    }
    // plain define (eg.: #define TRUE 1)
    else {
      DefinedMacros[DefinedID.GetString()] = {RemainingText, 0};
    }
  } else if (Directive.GetKind() == PPToken::Include) {
    assert(lexer.Is(PPToken::DoubleQuote) || lexer.Is(PPToken::LessThan));
    bool IsSystem = lexer.Is(PPToken::LessThan);
    lexer.Lex(); // eat the token

    std::string FileName;

    auto NextToken = lexer.Lex();
    while (NextToken.GetKind() == PPToken::Identifier ||
           NextToken.GetKind() == PPToken::Dot ||
           NextToken.GetKind() == PPToken::ForwardSlash) {
      FileName.append(NextToken.GetString());
      NextToken = lexer.Lex();
    }

    assert((NextToken.GetKind() == PPToken::DoubleQuote && !IsSystem) ||
           (NextToken.GetKind() == PPToken::GreaterThan && IsSystem));

    // in case if system headers were used, use source_code/include/ as include
    // path
    if (IsSystem) {
      auto Path = std::filesystem::current_path();
      Path.remove_filename();

      FilePath = Path;
      FilePath += "include/";
    }

    std::vector<std::string> FileContent;
    auto Success = getFileContent(FilePath + FileName, FileContent);
    assert(Success && "Cannot open file");

    Source.insert(Source.begin() + LineIdx + 1, FileContent.begin(),
                  FileContent.end());
  } else if (Directive.GetKind() == PPToken::IfNotDef) {
    // TODO
  } else if (Directive.GetKind() == PPToken::EndIf) {
    // TODO
  }
}

void PreProcessor::SubstituteMacros(std::string &Line) {
  // To save the processed line state at the beginning of the iteration.
  std::string LineCopy;

  do {
    LineCopy = Line;

    for (auto &[MacroID, MacroData] : DefinedMacros) {
      auto &[MacroBody, MacroParam] = MacroData;

      // simple search and replace of plain macros
      if (MacroParam == 0) {
        int LoopCounter = 0; // make sure not stuck in an endless loop

        while (LoopCounter < 20 && Line.find(MacroID) != std::string::npos) {
          Line.replace(Line.find(MacroID), MacroID.length(), MacroBody);
          LoopCounter++;
        }

        assert((LoopCounter < 20) && "Was stuck in an endless loop");
      }
      // otherwise, it is a function macro and have to substitute the right
      // values into its parameters
      else {
        auto Pos = Line.find(MacroID); // macro start pos

        // If it is the end of the line or the macro is a substring of another
        // identifier like example "MacroID == assert" and the line contain
        // "__assert_fail", then do not need to do substitution, continue with
        // next iteration.
        if (Pos == std::string::npos ||
            std::isalnum(Line[Pos + MacroID.length()]) ||
            Line[Pos + MacroID.length()] == '_' ||
            (Pos > 0 && (std::isalnum(Line[Pos - 1]) || Line[Pos - 1] == '_')))
          continue;

        Pos += MacroID.length();
        assert(Line[Pos] == '(');
        Pos++;

        auto RemainingLine = Line.substr(Pos);
        size_t StartPos = 0;
        std::vector<std::string> ActualParams(0);
        for (size_t i = 0; i < MacroParam; i++) {
          size_t EndPos = i != MacroParam - 1 ? RemainingLine.find(',')
                                              : RemainingLine.find(')');
          ActualParams.push_back(
              RemainingLine.substr(StartPos, EndPos - StartPos));
          StartPos = EndPos + 1;
        }

        auto ReplacedMacroBody = MacroBody;
        for (size_t i = 0; i < ActualParams.size(); i++) {
          auto Param = "$" + std::to_string(i);
          while (ReplacedMacroBody.find(Param) != std::string::npos) {
            // If a '#' character precede the parameter, then the content of
            // the parameter needs to be stringified.
            if (ReplacedMacroBody.find(Param) > 0 &&
                ReplacedMacroBody[ReplacedMacroBody.find(Param) - 1] == '#')
              ReplacedMacroBody.replace(ReplacedMacroBody.find(Param) - 1,
                                        Param.length() + 1,
                                        "\"" + ActualParams[i] + "\"");
            else // Plain string replace
              ReplacedMacroBody.replace(ReplacedMacroBody.find(Param),
                                        Param.length(), ActualParams[i]);
          }
        }

        if (Line.find(MacroID) != std::string::npos)
          Line.replace(Line.find(MacroID), MacroID.length() + StartPos + 1,
                       ReplacedMacroBody);
      }
    }

  } while (LineCopy != Line);
}

void PreProcessor::Run() {
  for (size_t LineIdx = 0; LineIdx < Source.size(); LineIdx++) {
    auto &Line = Source[LineIdx];
    if (Line.empty())
      continue;

    if (!Line.empty() && Line[0] == '#') {
      ParseDirective(Line, LineIdx);
      // delete current line, assuming the directive only used one line
      Source.erase(Source.begin() + LineIdx);
      LineIdx--; // since the erase we have to check again the same LineIdx
    } else if (!DefinedMacros.empty()) {
      // Update __LINE__ here, so the correct line number can be substituted
      // TODO: With the current handling of the includes - which is basically
      // insertion of its content - this macro will return false line numbers
      // if any includes were used. Fix it.
      DefinedMacros["__LINE__"].first = std::to_string(LineIdx + 1);
      SubstituteMacros(Line);
    }
  }
}
