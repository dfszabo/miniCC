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

  uint64_t GetIntValue() { return std::get<uint64_t>(Val); }

  std::string ValueString() const override {
    if (IsFPConst())
      return std::to_string(std::get<double>(Val));
    else
      return std::to_string(std::get<uint64_t>(Val));
  }

private:
  std::variant<uint64_t, double> Val;
};

class FunctionParameter : public Value {
public:
  FunctionParameter() = delete;
  FunctionParameter(std::string &Name, IRType Type)
      : Value(PARAM, Type), Name(Name) {}

  std::string &GetName() { return Name; }
  std::string ValueString() const override { return "$" + Name; }

private:
  std::string Name;
};

class GlobalVariable : public Value {
public:
  GlobalVariable() = delete;
  GlobalVariable(std::string &Name, IRType Type)
      : Value(GLOBALVAR, Type), Name(Name) {}

  std::string ValueString() const override { return "@" + Name; }

  void Print() const {
    std::cout << "global " << Name << " :" << GetType().AsString() << ";"
              << std::endl
              << std::endl;
  }

private:
  std::string Name;
};

#endif
