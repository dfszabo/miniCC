#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "../ast/Type.hpp"
#include "../lexer/Token.hpp"
#include <cassert>
#include <optional>
#include <string>
#include <vector>

/// Holds a stack (vector) of SymbolTables (vector of tuples).
/// TODO: An unordered_map would be more appropriate, but
/// had trouble with it so rather than wasting too much time
/// with it currently vectors used. Later improve this.
class SymbolTableStack {
public:
  using Entry = std::tuple<Token, Type, ValueType>;
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

  void InsertGlobalEntry(const Entry &e) { SymTabStack[0].push_back(e); }

  std::optional<Entry> Contains(const std::string &sym);

  std::optional<Entry> ContainsInCurrentScope(const std::string &sym);

  std::optional<Entry> ContainsInGlobalScope(const std::string &sym);

private:
  std::vector<Table> SymTabStack;
};

#endif