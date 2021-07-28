#ifndef VALUE_HPP
#define VALUE_HPP

#include "Type.hpp"
#include <iostream>
#include <string>
#include <variant>

class Value {
public:
  enum VKind { INVALID = 1, NONE, REGISTER, LABEL, CONST, PARAM, GLOBALVAR };

  Value() : Kind(INVALID) {}
  Value(VKind VK) : Kind(VK) {}
  Value(IRType T) : ValueType(T), Kind(REGISTER) {}
  Value(VKind VK, IRType T) : Kind(VK), ValueType(T) {}

  virtual ~Value(){};

  IRType &GetTypeRef() { return ValueType; }
  IRType GetType() const { return ValueType; }

  unsigned GetID() const { return UniqeID; }
  void SetID(const unsigned i) { UniqeID = i; }

  unsigned GetBitWidth() const { return ValueType.GetBitSize(); }

  bool IsConstant() const { return Kind == CONST; }
  bool IsRegister() const { return Kind == REGISTER; }
  bool IsParameter() const { return Kind == PARAM; }
  bool IsGlobalVar() const { return Kind == GLOBALVAR; }

  bool IsIntType() const { return ValueType.IsINT(); }

  virtual std::string ValueString() const {
    return "$" + std::to_string(UniqeID) + "<" + ValueType.AsString() + ">";
  }

protected:
  unsigned UniqeID;
  VKind Kind = REGISTER;
  IRType ValueType;
};

class Constant : public Value {
public:
  Constant() = delete;
  Constant(uint64_t V) : Value(Value::CONST, IRType::UINT), Val(V) {}
  Constant(double V) : Value(Value::CONST, IRType(IRType::FP, 64)), Val(V) {}

  bool IsFPConst() const { return ValueType.IsFP(); }

  uint64_t GetIntValue() {
    assert(ValueType.IsINT());
    return std::get<uint64_t>(Val);
  }

  std::string ValueString() const override {
    if (IsFPConst())
      return std::to_string(std::get<double>(Val));
    else
      return std::to_string((int64_t)std::get<uint64_t>(Val));
  }

private:
  std::variant<uint64_t, double> Val;
};

class FunctionParameter : public Value {
public:
  FunctionParameter() = delete;
  FunctionParameter(std::string &Name, IRType Type, bool Struct = false)
      : Value(PARAM, Type), Name(Name), ImplicitStructPtr(Struct) {}

  std::string &GetName() { return Name; }
  bool IsImplicitStructPtr() const { return ImplicitStructPtr; }
  std::string ValueString() const override { return "$" + Name; }

private:
  std::string Name;
  bool ImplicitStructPtr = false;
};

class GlobalVariable : public Value {
public:
  GlobalVariable() = delete;
  GlobalVariable(std::string &Name, IRType Type)
      : Value(GLOBALVAR, Type), Name(Name) {}

  GlobalVariable(std::string &Name, IRType Type, std::vector<uint64_t> InitList)
      : Value(GLOBALVAR, Type), Name(Name),
        InitList(std::move(InitList)) {}

  std::string &GetName() { return Name; }
  std::vector<uint64_t> &GetInitList() { return InitList; }

  std::string ValueString() const override {
    return "@" + Name + "<" + ValueType.AsString() + ">";
  }

  void Print() const {
    std::cout << "global var (" << GetType().AsString() << "):" << std::endl
              << "\t" << Name;

    if (!InitList.empty()) {
      std::cout << " = {";
      for (size_t i = 0; i < InitList.size(); i++) {
        std::cout << " " << std::to_string(InitList[i]);
        if (i  + 1 < InitList.size())
          std::cout << ",";
      }
      std::cout << " }";
    }

    std::cout << std::endl << std::endl;
  }

private:
  std::string Name;
  std::vector<uint64_t> InitList;
};

#endif
