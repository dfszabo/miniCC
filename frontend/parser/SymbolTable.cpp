#include "SymbolTable.hpp"
#include <tuple>


std::optional<SymbolTableStack::Entry>
SymbolTableStack::Contains(const std::string &sym) {
  for (int i = Size() - 1; i >= 0; i--) {
    auto table = SymTabStack[i];
    for (int j = table.size() - 1; j >= 0; j--)
      if (sym == std::get<0>(table[j]).GetString())
        return table[j];
  }
  return std::nullopt;
}

std::optional<SymbolTableStack::Entry>
SymbolTableStack::ContainsInCurrentScope(const std::string &sym) {
  auto table = SymTabStack.back();
  for (int j = table.size() - 1; j >= 0; j--)
    if (sym == std::get<0>(table[j]).GetString())
      return table[j];

  return std::nullopt;
}

std::optional<SymbolTableStack::Entry>
SymbolTableStack::ContainsInGlobalScope(const std::string &sym) {
  auto table = SymTabStack[0];
  for (int j = table.size() - 1; j >= 0; j--)
    if (sym == std::get<0>(table[j]).GetString())
      return table[j];

  return std::nullopt;
}
