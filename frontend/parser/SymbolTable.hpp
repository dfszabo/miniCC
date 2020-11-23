#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "../ast/Type.hpp"
#include <cassert>
#include <optional>
#include <string>
#include <vector>

/// Holds a stack (vector) of SymbolTables (vector of tuples).
/// FIXME: An unordered_map would be more appropriate, but
/// had trouble with it so rather than wasting too much time
/// with it curently vectors used. Later improve this.
class SymbolTableStack {
public:
  using Entry = std::tuple<std::string, ComplexType, ValueType>;
  using Table = std::vector<Entry>;

  /// Adding the first empty table when constructed
  SymbolTableStack() { SymTabStack.push_back(Table()); }

  void PushSymTable(Table t = Table()) { SymTabStack.push_back(std::move(t)); }

  Table PopSymTable() {
    assert(SymTabStack.size() > 0 && "Popping item from empty stack.");
    Table t = SymTabStack[SymTabStack.size() - 1];
    SymTabStack.pop_back();
    return t;
  }

  size_t Size() { return SymTabStack.size(); }

  Table &GetTopTable() { return SymTabStack.back(); }

  void InsertEntry(const Entry &e) {
    auto idx = Size() > 0 ? Size() - 1 : 0;
    SymTabStack[idx].push_back(e);
  }

  bool Contains(Entry e) {
    for (int i = Size() - 1; i >= 0; i--) {
      auto table = SymTabStack[i];
      for (int j = table.size() - 1; j >= 0; j--)
        if (e == table[j])
          return true;
    }
    return false;
  }

  std::optional<Entry> Contains(const std::string &sym) {
    for (int i = Size() - 1; i >= 0; i--) {
      auto table = SymTabStack[i];
      for (int j = table.size() - 1; j >= 0; j--)
        if (sym == std::get<0>(table[j]))
          return table[j];
    }
    return std::nullopt;
  }

  bool ContainsInCurrentScope(Entry e) {
    auto idx = Size() > 0 ? Size() - 1 : 0;

    for (int i = SymTabStack[idx].size() - 1; i >= 0; i--)
      if (e == SymTabStack[idx][i])
        return true;
    return false;
  }

private:
  std::vector<Table> SymTabStack;
};

#endif