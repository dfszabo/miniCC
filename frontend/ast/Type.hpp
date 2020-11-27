#ifndef TYPE_HPP
#define TYPE_HPP

#include <cassert>
#include <vector>

class Type {
public:
  /// Basic type variants. Numerical ones are ordered by conversion rank.
  enum VariantKind { Invalid, Void, Int, Double };

  VariantKind GetTypeVariant() const { return Ty; }
  void SetTypeVariant(VariantKind t) { Ty = t; }

  virtual std::string ToString() const { return ToString(Ty); }

  static std::string ToString(const VariantKind vk) {
    switch (vk) {
    case Double:
      return "double";
    case Int:
      return "int";
    case Void:
      return "void";
    case Invalid:
      return "invalid";
    default:
      assert(false && "Unknown type.");
      break;
    }
  }

  Type() : Ty(Invalid) {}
  Type(VariantKind vk) : Ty(vk) {}

  Type(Type &&) = default;
  Type &operator=(Type &&) = default;

  Type(const Type &) = default;
  Type &operator=(const Type &) = default;

  // FIXME: This can be deleted I believe.
  virtual ~Type() = default;

  /// Given two type variants it return the stronger one.
  /// Type variants must be numerical ones.
  /// Example Int and Double -> result Double.
  static Type GetStrongestType(const Type::VariantKind type1,
                               const Type::VariantKind type2) {
    if (type1 > type2)
      return type1;
    else
      return type2;
  }

  static bool IsImplicitlyCastable(const Type::VariantKind from,
                                   const Type::VariantKind to) {
    return (from == Int && to == Double) || (from == Double && to == Int);
  }

protected:
  VariantKind Ty;
};

class ArrayType : public Type {
public:
  ArrayType() = default;
  ArrayType(Type t) : Type(t) {}
  ArrayType(Type t, std::vector<unsigned> d) : Type(t), Dimensions(d) {}

  ArrayType(ArrayType &&) = default;
  ArrayType &operator=(ArrayType &&) = default;

  ArrayType(const ArrayType &) = default;
  ArrayType &operator=(const ArrayType &) = default;

  std::vector<unsigned> &GetDimensions() { return Dimensions; }
  void SetDimensions(std::vector<unsigned> d) { Dimensions = std::move(d); }

  std::string ToString() const override {
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
  FunctionType(Type t, std::vector<VariantKind> a)
      : Type(t), ArgumentTypes(a) {}

  FunctionType(FunctionType &&) = default;
  FunctionType &operator=(FunctionType &&) = delete;

  FunctionType(const FunctionType &) = default;
  FunctionType &operator=(const FunctionType &) = default;

  std::vector<VariantKind> &GetArgumentTypes() { return ArgumentTypes; }
  void SetArgumentTypes(std::vector<VariantKind> &v) { ArgumentTypes = v; }
  void AddArgumentType(VariantKind v) { ArgumentTypes.push_back(v); }

  VariantKind GetReturnType() const { return GetTypeVariant(); }
  void SetReturnType(VariantKind v) { SetTypeVariant(v); }

  std::string ToString() const override {
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

// For now make this a separate type but it should be the only type
class ComplexType : public Type {
public:
  ComplexType() : Kind(Simple), Type(Invalid) {}
  ComplexType(Type t) {
    Kind = Simple;
    Ty = t.GetTypeVariant();
  }
  ComplexType(Type::VariantKind vk) {
    Kind = Simple;
    Ty = vk;
  }
  ComplexType(Type t, std::vector<unsigned> d) {
    if (d.size() == 0)
      ComplexType(t);
    else {
      Kind = Array;
      Dimensions = std::move(d);
    }
    Ty = t.GetTypeVariant();
  }
  ComplexType(Type t, std::vector<Type::VariantKind> a) {
    Kind = Function;
    ArgumentTypes = std::move(a);
    Ty = t.GetTypeVariant();
  }
  ComplexType(ArrayType t)
      : Type(t.GetTypeVariant()), Kind(Array), Dimensions(t.GetDimensions()) {}
  ComplexType(FunctionType t)
      : Type(t.GetReturnType()), Kind(Function),
        ArgumentTypes(t.GetArgumentTypes()) {}

  ComplexType(ComplexType &&ct) {
    Ty = ct.Ty;
    Kind = ct.Kind;
    if (ct.Kind == Array)
      Dimensions = std::move(ct.Dimensions);
    else if (ct.Kind == Function)
      ArgumentTypes = std::move(ct.ArgumentTypes);
  }

  ComplexType &operator=(ComplexType &&ct) {
    Ty = ct.Ty;
    Kind = ct.Kind;
    if (ct.Kind == Array)
      Dimensions = std::move(ct.Dimensions);
    else if (ct.Kind == Function)
      ArgumentTypes = std::move(ct.ArgumentTypes);
  }

  ComplexType(const ComplexType &ct) {
    Ty = ct.Ty;
    Kind = ct.Kind;
    if (ct.Kind == Array)
      Dimensions = ct.Dimensions;
    else if (ct.Kind == Function)
      ArgumentTypes = ct.ArgumentTypes;
  }

  ComplexType &operator=(const ComplexType &ct) {
    Ty = ct.Ty;
    Kind = ct.Kind;
    if (ct.Kind == Array)
      Dimensions = ct.Dimensions;
    else if (ct.Kind == Function)
      ArgumentTypes = ct.ArgumentTypes;
  }

  bool IsSimpleType() { return Kind == Simple; }
  bool IsArrayType() { return Kind == Array; }
  bool IsFunctionType() { return Kind == Function; }

  Type GetType() { return Type(Ty); }
  ArrayType GetArrayType() { return ArrayType(Ty, Dimensions); }
  FunctionType GetFunctionType() { return FunctionType(Ty, ArgumentTypes); }

  std::vector<unsigned> &GetDimensions() {
    assert(IsArrayType() && "Must be an Array type to access Dimensions.");
    return Dimensions;
  }

  std::vector<Type::VariantKind> &GetArgTypes() {
    assert(IsFunctionType() &&
           "Must be a Function type to access ArgumentTypes.");
    return ArgumentTypes;
  }

  friend bool operator==(const ComplexType &lhs, const ComplexType &rhs) {
    bool result = lhs.Kind == rhs.Kind && lhs.Ty == rhs.Ty;

    if (!result)
      return false;

    switch (lhs.Kind) {
    case Function:
      result = result && (lhs.ArgumentTypes == rhs.ArgumentTypes);
      break;
    case Array:
      result = result && (lhs.Dimensions == rhs.Dimensions);
      break;
    default:
      break;
    }
    return result;
  }

  std::string ToString() const override {
    if (Kind == Function) {
      auto TyStr = Type::ToString(Ty);
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
    } else if (Kind == Array) {
      auto TyStr = Type::ToString(Ty);

      for (int i = 0; i < Dimensions.size(); i++)
        TyStr += "[" + std::to_string(Dimensions[i]) + "]";
      return TyStr;
    } else {
      return Type::ToString(Ty);
    }
  }

private:
  enum TypeKind { Simple, Array, Function };
  TypeKind Kind;
  // TODO: revisit the use of union, deleted from here since
  // it just made things complicated
  std::vector<VariantKind> ArgumentTypes;
  std::vector<unsigned> Dimensions;
};

// Hold an integer or a float value
class ValueType {
public:
  ValueType() : Kind(Empty) {}
  ValueType(unsigned v) : Kind(Integer), IntVal(v) {}
  ValueType(double v) : Kind(Float), FloatVal(v) {}

  ValueType(ValueType &&) = default;
  ValueType &operator=(ValueType &&) = delete;

  ValueType(const ValueType &) = default;
  ValueType &operator=(const ValueType &) = delete;

  bool IsInt() { return Kind == Integer; }
  bool IsFloat() { return Kind == Float; }
  bool IsEmpty() { return Kind == Empty; }

  unsigned GetIntVal() {
    assert(IsInt() && "Must be an integer type.");
    return IntVal;
  }
  double GetFloatVal() {
    assert(IsFloat() && "Must be a float type.");
    return FloatVal;
  }

  friend bool operator==(const ValueType &lhs, const ValueType &rhs) {
    bool result = lhs.Kind == rhs.Kind;

    if (!result)
      return false;

    switch (lhs.Kind) {
    case Integer:
      result = result && (lhs.IntVal == rhs.IntVal);
      break;
    case Float:
      result = result && (lhs.FloatVal == rhs.FloatVal);
      break;
    default:
      break;
    }
    return result;
  }

private:
  enum ValueKind { Empty, Integer, Float };
  ValueKind Kind;
  union {
    unsigned IntVal;
    double FloatVal;
  };
};

#endif