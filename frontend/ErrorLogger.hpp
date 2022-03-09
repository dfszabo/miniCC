#ifndef ERROR_LOGGER_H
#define ERROR_LOGGER_H

#include <iostream>
#include <string>
#include <vector>

class ErrorLogger {
public:
  ErrorLogger(const std::string &FileName) : FileName(FileName) {}

  void AddError(const std::string &Msg) { ErrorMessages.push_back(Msg); }

  bool HasErrors() const { return ErrorMessages.size() > 0; }

  void ReportErrors() const {
    for (auto &Msg : ErrorMessages)
      std::cout << FileName << Msg << std::endl << std::endl;
  }

private:
  std::string FileName;
  std::vector<std::string> ErrorMessages;
};

#endif
