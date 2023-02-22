#include "ErrorLogger.hpp"

std::string CreateCodePointerString(const Token &T) {
  size_t StartIdx = T.GetColNum();
  std::string Spaces(StartIdx, ' ');
  std::string Hats(T.GetString().length(), '^');
  return Spaces + Hats;
}

void ErrorLogger::AddMessage(const std::string &Msg) {
  ErrorMessages.push_back(Msg);
}

void ErrorLogger::AddMessage(const std::string &Msg, const char *Type) {
  ErrorMessages.push_back(std::string(": ") + Type + std::string(": ") + Msg);
}

void ErrorLogger::AddMessage(const std::string &Msg, const char *Type,
                             const Token &T) {
  assert(T.GetLineNum() < Source.size() && "Out of bound index");
  std::string MsgWithLineNums = ":" + std::to_string(T.GetLineNum() + 1) + ":" +
                                std::to_string(T.GetColNum() + 1) +
                                std::string(": ") + Type + std::string(": ") +
                                Msg + "\n" + Source[T.GetLineNum()] + "\n" +
                                CreateCodePointerString(T);
  AddMessage(MsgWithLineNums);
}

void ErrorLogger::AddError(const std::string &Msg) {
  HasError = true;
  AddMessage(Msg, "error");
}

void ErrorLogger::AddError(const std::string &Msg, const Token &T) {
  HasError = true;
  AddMessage(Msg, "error", T);
}

void ErrorLogger::AddWarning(const std::string &Msg) {
  HasWarning = true;
  AddMessage(Msg, "warning");
}

void ErrorLogger::AddWarning(const std::string &Msg, const Token &T) {
  HasWarning = true;
  AddMessage(Msg, "warning", T);
}

void ErrorLogger::AddNote(const std::string &Msg) {
  AddMessage(Msg, "note");
}

void ErrorLogger::AddNote(const std::string &Msg, const Token &T) {
  AddMessage(Msg, "note", T);
}

bool ErrorLogger::HasErrors(const bool Wall) const {
  return Wall ? HasError || HasWarning : HasError;
}

void ErrorLogger::ReportErrors() const {
  for (auto &Msg : ErrorMessages)
    std::cout << FileName << Msg << std::endl;
}
