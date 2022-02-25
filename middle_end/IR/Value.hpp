#ifndef VALUE_HPP
#define VALUE_HPP

#include "IRType.hpp"
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

  void SetType(IRType t) { ValueType = t; }

  unsigned GetID() const { return UniqeID; }
  void SetID(const unsigned i) { UniqeID = i; }

  unsigned GetBitWidth() const { return ValueType.GetBitSize(); }

  bool IsConstant() const { return Kind == CONST; }
  bool IsRegister() const { return Kind == REGISTER; }
  bool IsParameter() const { return Kind == PARAM; }
  bool IsGlobalVar() const { return Kind == GLOBALVAR; }

  bool IsIntType() const { return ValueType.IsINT(); }
  bool IsFPType() const { return ValueType.IsFP(); }

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
  Constant(uint64_t V, uint8_t BW = 32)
      : Value(Value::CONST, IRType(IRType::UINT, BW)), Val(V) {}
  Constant(double V) : Value(Value::CONST, IRType(IRType::FP, 64)), Val(V) {}

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

  GlobalVariable(std::string &Name, IRType Type, std::string InitStr)
      : Value(GLOBALVAR, Type), Name(Name), InitString(InitStr) {}

  GlobalVariable(std::string &Name, IRType Type, Value *InitValue)
      : Value(GLOBALVAR, Type), Name(Name), InitValue(InitValue) {}

  GlobalVariable(std::string &Name, IRType Type, std::vector<uint64_t> InitList)
      : Value(GLOBALVAR, Type), Name(Name),
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
