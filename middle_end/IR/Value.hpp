#ifndef VALUE_HPP
#define VALUE_HPP

#include "IRType.hpp"
#include <iostream>
#include <string>
#include <utility>
#include <variant>

class Value {
public:
  enum VKind { INVALID = 1, NONE, REGISTER, LABEL, CONST, PARAM, GLOBALVAR };

  Value() : Kind(INVALID) {}
  explicit Value(VKind VK) : Kind(VK) {}
  explicit Value(IRType T) : ValueType(std::move(T)), Kind(REGISTER) {}
  Value(VKind VK, IRType T) : Kind(VK), ValueType(std::move(T)) {}

  virtual ~Value() = default;

  IRType &GetTypeRef() { return ValueType; }
  IRType GetType() const { return ValueType; }

  unsigned GetID() const { return UniqueID; }
  void SetID(const unsigned i) { UniqueID = i; }

  unsigned GetBitWidth() const { return ValueType.GetBitSize(); }

  bool IsConstant() const { return Kind == CONST; }
  bool IsRegister() const { return Kind == REGISTER; }
  bool IsParameter() const { return Kind == PARAM; }
  bool IsGlobalVar() const { return Kind == GLOBALVAR; }

  bool IsIntType() const { return ValueType.IsINT(); }
  bool IsFPType() const { return ValueType.IsFP(); }

  virtual std::string ValueString() const {
    return "$" + std::to_string(UniqueID) + "<" + ValueType.AsString() + ">";
  }

protected:
  unsigned UniqueID = ~0;
  VKind Kind = REGISTER;
  IRType ValueType;
};

class Constant : public Value {
public:
  Constant() = delete;
  explicit Constant(uint64_t V, uint8_t BW = 32)
      : Value(Value::CONST, IRType(IRType::UINT, BW)), Val(V) {}
  explicit Constant(double V, uint8_t BW = 32)
      : Value(Value::CONST, IRType(IRType::FP, BW)), Val(V) {}

  bool IsFPConst() const { return ValueType.IsFP(); }

  uint64_t GetIntValue() const;
  double GetFloatValue() const;

  std::string ValueString() const override;

private:
  std::variant<uint64_t, double> Val;
};

class FunctionParameter : public Value {
public:
  FunctionParameter() = delete;
  FunctionParameter(std::string &Name, IRType Type, bool Struct = false)
      : Value(PARAM, std::move(Type)), Name(Name), ImplicitStructPtr(Struct) {}

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
      : Value(GLOBALVAR, std::move(Type)), Name(Name) {}

  GlobalVariable(std::string &Name, IRType Type, std::string InitStr)
      : Value(GLOBALVAR, std::move(Type)), Name(Name),
        InitString(std::move(InitStr)) {}

  GlobalVariable(std::string &Name, IRType Type, Value *InitValue)
      : Value(GLOBALVAR, std::move(Type)), Name(Name), InitValue(InitValue) {}

  GlobalVariable(std::string &Name, IRType Type, std::vector<uint64_t> InitList)
      : Value(GLOBALVAR, std::move(Type)), Name(Name),
        InitList(std::move(InitList)) {}

  std::string &GetName() { return Name; }
  std::vector<uint64_t> &GetInitList() { return InitList; }
  std::string &GetInitString() { return InitString; }
  Value *GetInitValue() { return InitValue; }

  std::string ValueString() const override {
    return "@" + Name + "<" + ValueType.AsString() + ">";
  }

  void Print() const;

private:
  std::string Name;
  std::vector<uint64_t> InitList;
  std::string InitString;
  Value *InitValue = nullptr;
};

#endif
