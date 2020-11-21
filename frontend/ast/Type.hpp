#ifndef TYPE_HPP
#define TYPE_HPP

#include <cassert>
#include <vector>

class Type {
public:
  enum VariantKind { Double, Int, Void };

  VariantKind GetTypeVariant() { return Ty; }
  void SetTypeVariant(VariantKind t) { Ty = t; }

  virtual std::string ToString() { return ToString(Ty); }

  static std::string ToString(VariantKind vk) {
    switch (vk) {
    case Double:
      return "double";
    case Int:
      return "int";
    case Void:
      return "void";
    default:
      assert(false && "Unknown type.");
      break;
    }
  }

  Type() = default;
  Type(VariantKind vk) : Ty(vk) {}
  Type(Type&&) = default;
  Type &operator=(Type&&) = default;
  Type(const Type&) = default;
  Type &operator=(const Type&) = default;

protected:
  VariantKind Ty;
};

class ArrayType : public Type {
public:
  ArrayType() = default;
  ArrayType(Type t) : Type(t) {}
  ArrayType(Type t, std::vector<unsigned> d) : Type(t), Dimensions(d) {}

  std::vector<unsigned> &GetDimensions() { return Dimensions; }
  void SetDimensions(std::vector<unsigned> d) { Dimensions = std::move(d); }

  std::string ToString() override {
    auto TyStr = Type::ToString(Ty);

    for (int i = 0; i < Dimensions.size(); i++)
      TyStr += "[" + std::to_string(Dimensions[i]) + "]";
    return TyStr;
  }

private:
  std::vector<unsigned> Dimensions;
};

class FunctionType : public Type {
public:
  FunctionType() = default;
  FunctionType(Type t) : Type(t) {}
  std::vector<VariantKind> &GetArgumentTypes() { return ArgumentTypes; }
  void SetArgumentTypes(std::vector<VariantKind> &v) { ArgumentTypes = v; }
  void AddArgumentType(VariantKind v) { ArgumentTypes.push_back(v); }
  VariantKind GetReturnType() { return GetTypeVariant(); }
  void SetReturnType(VariantKind v) { SetTypeVariant(v); }

  std::string ToString() override {
    auto TyStr = Type::ToString(GetReturnType());
    auto ArgSize = ArgumentTypes.size();
    if (ArgSize > 0)
      TyStr += " (";
    for (int i = 0; i < ArgSize; i++) {
      TyStr += Type::ToString(ArgumentTypes[i]);
      if (i + 1 < ArgSize)
        TyStr += ",";
      else
        TyStr += ")";
    }
    return TyStr;
  }

private:
  std::vector<VariantKind> ArgumentTypes;
};

#endif