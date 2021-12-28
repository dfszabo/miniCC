#include "SymbolTable.hpp"
#include <tuple>

bool SymbolTableStack::Contains(SymbolTableStack::Entry e) {
  for (int i = Size() - 1; i >= 0; i--) {
    auto table = SymTabStack[i];
    for (int j = table.size() - 1; j >= 0; j--)
      if (e == table[j])
        return true;
  }
  return false;
}

std::optional<SymbolTableStack::Entry>
SymbolTableStack::Contains(const std::string &sym) {
  for (int i = Size() - 1; i >= 0; i--) {
    auto table = SymTabStack[i];
    for (int j = table.size() - 1; j >= 0; j--)
      if (sym == std::get<0>(table[j]))
        return table[j];
  }
  return std::nullopt;
}

bool SymbolTableStack::ContainsInCurrentScope(SymbolTableStack::Entry e) {
  auto idx = Size() > 0 ? Size() - 1 : 0;

  for (int i = SymTabStack[idx].size() - 1; i >= 0; i--)
    if (e == SymTabStack[idx][i])
      return true;
  return false;
}
