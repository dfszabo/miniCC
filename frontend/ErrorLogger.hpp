#ifndef ERROR_LOGGER_H
#define ERROR_LOGGER_H

#include "lexer/Token.hpp"
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class ErrorLogger {
public:
  ErrorLogger(std::string FileName, std::vector<std::string> Source)
      : FileName(std::move(FileName)), Source(std::move(Source)) {}

  void AddMessage(const std::string &Msg);
  void AddMessage(const std::string &Msg, const char *Type);
  void AddMessage(const std::string &Msg, const char *Type, const Token &T);

  void AddError(const std::string &Msg);
  void AddError(const std::string &Msg, const Token &T);

  void AddWarning(const std::string &Msg);
  void AddWarning(const std::string &Msg, const Token &T);

  void AddNote(const std::string &Msg);
  void AddNote(const std::string &Msg, const Token &T);

  bool HasErrors(bool Wall = false) const;

  void ReportErrors() const;

private:
  std::string FileName;
  const std::vector<std::string> Source;
  std::vector<std::string> ErrorMessages;
  bool HasError = false;
  bool HasWarning = false;
};

#endif
