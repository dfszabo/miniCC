#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <map>
#include <string>
#include <vector>

class PreProcessor {
public:
  PreProcessor() = delete;
  PreProcessor(std::vector<std::string> &Src) : Source(Src) {}

  void ParseDirective(std::string &Line);
  void SubstituteMacros(std::string &Line);
  void Run();

private:
  std::vector<std::string> &Source;
  std::map<std::string, std::string> DefinedMacros;
};

#endif
