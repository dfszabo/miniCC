#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <map>
#include <string>
#include <vector>

class PreProcessor {
public:
  PreProcessor() = delete;
  PreProcessor(std::vector<std::string> &Src, const std::string &Path)
      : Source(Src) {
    FilePath = Path.substr(0, Path.rfind('/'));
    if (FilePath.length() > 0 && FilePath[FilePath.length() - 1] != '/')
      FilePath.push_back('/');

    DefinedMacros["__FILE__"] = {"\"" + Path + "\"", 0};
    DefinedMacros["__LINE__"] = {"1", 0};
  }

  void ParseDirective(std::string &Line, size_t LineIdx);
  void SubstituteMacros(std::string &Line);
  void Run();

private:
  std::string FilePath;
  std::vector<std::string> &Source;
  std::map<std::string, std::pair<std::string, unsigned>> DefinedMacros;
};

#endif
