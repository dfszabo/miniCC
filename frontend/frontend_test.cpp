#include "lexer/Lexer.hpp"
#include "parser/Parser.hpp"
#include "parser/SymbolTable.hpp"
#include "../middle_end/IR/IRFactory.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*
 * It will iterate through all the lines in file and
 * put them in given vector
 */
bool getFileContent(std::string fileName, std::vector<std::string> &vecOfStrs) {
  // Open the File
  std::ifstream in(fileName.c_str());
  // Check if object is valid
  if (!in) {
    std::cerr << "Cannot open the File : " << fileName << std::endl;
    return false;
  }
  std::string str;
  // Read the next line from File untill it reaches the end.
  while (std::getline(in, str)) {
    // Line contains string of length > 0 then save it in vector
    vecOfStrs.push_back(str);
  }
  // Close The File
  in.close();
  return true;
}

int main(int argc, char *argv[]) {
  std::string FilePath = "tests/test.txt";
  bool DumpTokens = false;
  bool DumpAST = false;
  bool DumpIR = false;

  for (int i = 0; i < argc; i++)
    if (argv[i][0] != '-')
      FilePath = std::string(argv[i]);
    else {
      if (!std::string(&argv[i][1]).compare("dump-tokens")) {
        DumpTokens = true;
        continue;
      }
      if (!std::string(&argv[i][1]).compare("dump-ast")) {
        DumpAST = true;
        continue;
      }
      if (!std::string(&argv[i][1]).compare("dump-ir")) {
        DumpIR = true;
        continue;
      }
    }

  if (DumpTokens) {
    std::vector<std::string> src;
    getFileContent(FilePath.c_str(), src);

    Lexer lexer(src);

    auto t1 = lexer.Lex();
    while (t1.GetKind() != Token::EndOfFile && t1.GetKind() != Token::Invalid) {
      std::cout << t1.ToString() << std::endl;
      t1 = lexer.Lex();
    }
  }

  std::vector<std::string> src;
  getFileContent(FilePath.c_str(), src);

  Module IRModule;
  IRFactory IRF(IRModule); 
  Parser parser(src, &IRF);
  auto AST = parser.Parse();
  if (DumpAST)
    AST->ASTDump();
  if (DumpIR)
    AST->IRCodegen(&IRF), IRModule.Print();

  return 0;
}