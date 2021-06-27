#ifndef TYPE_HPP
#define TYPE_HPP

#include <cassert>
#include <string>
#include <vector>

class Type {
public:
  /// Basic type variants. Numerical ones are ordered by conversion rank.
  enum VariantKind { Invalid, Composite, Void, Char,
                     UnsignedChar, Int, UnsignedInt, Double };
  enum TypeKind { Simple, Array, Struct };
  enum TypeQualifier : unsigned { None, Typedef, Const };

  std::string GetName() const { return Name; }
  void SetName(std::string &n) { Name = n; }

  TypeKind GetTypeKind() const { return Kind; }
  void SetTypeKind(TypeKind t) { Kind = t; }

  VariantKind GetTypeVariant() const { return Ty; }
  void SetTypeVariant(VariantKind t) { Ty = t; }

  unsigned GetQualifiers() const { return Qualifiers; }
  void SetQualifiers(unsigned q) { Qualifiers = q; }
  void AddQualifier(unsigned q) { Qualifiers |= q; }

  uint8_t GetPointerLevel() const { return PointerLevel; }
  void SetPointerLevel(uint8_t pl) { PointerLevel = pl; }
  void IncrementPointerLevel() { PointerLevel++; }
  void DecrementPointerLevel() {
    assert(PointerLevel > 0 && "Cannot decrement below 0");
    PointerLevel--;
  }

  bool IsPointerType() const { return PointerLevel != 0; }

  static std::string ToString(const Type *t) {
    switch (t->GetTypeVariant()) {
    case Double:
      return "double";
    case Char:
      return "char";
    case UnsignedChar:
      return "unsigned char";
    case Int:
      return "int";
    case UnsignedInt:
      return "unsigned int";
    case Void:
      return "void";
    case Composite:
      return t->GetName();
    case Invalid:
      return "invalid";
    default:
      assert(false && "Unknown type.");
      break;
    }
  }

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
    return (from == Int && to == Double) || (from == Double && to == Int) ||
           ((from == Char || from == UnsignedChar) && to == Int) ||
           (from == Int && (to == Char || to == UnsignedChar));
  }

  static bool IsSmallerThanInt(const Type::VariantKind v) {
    switch (v) {
    case Char:
    case UnsignedChar:
      return true;
    default:
      return false;
    }
  }

  static bool OnlySigndnessDifference(const Type::VariantKind v1,
                                      const Type::VariantKind v2) {
    if ((v1 == Int && v2 == UnsignedInt) || (v2 == Int && v1 == UnsignedInt) ||
        (v1 == Char && v2 == UnsignedChar) || (v2 == Char && v1 == UnsignedChar))
      return true;

    return false;
  }

  Type() : Kind(Simple), Ty(Invalid) {}
  Type(TypeKind tk)  { Kind = tk;
    switch (tk) {
    case Array:
    case Struct:
      Ty = Composite;
      break;
    case Simple:
    default:
      Ty = Invalid;
      break;
    }
  }
  Type(VariantKind vk) {
    Kind = Simple;
    Ty = vk;
  }

  Type(Type t, std::vector<unsigned> d) : Type(t) {
    if (d.empty()) {
      Kind = Simple;
    } else {
      Kind = Array;
      Dimensions = std::move(d);
    }
  }

  Type(Type t, std::vector<Type> a) {
    ParameterList = std::move(a);
    Ty = t.GetTypeVariant();
  }

  Type(Type &&ct) {
    PointerLevel = ct.PointerLevel;
    Ty = ct.Ty;
    Kind = ct.Kind;
    Dimensions = std::move(ct.Dimensions);
    TypeList = std::move(ct.TypeList);
    ParameterList = std::move(ct.ParameterList);
    Name = ct.Name;
  }

  Type(const Type &ct) = default;
  Type &operator=(const Type &ct) = default;

  bool IsSimpleType() const { return Kind == Simple; }
  bool IsArray() const { return Kind == Array; }
  bool IsFunction() const { return ParameterList.size() > 0; }
  bool IsStruct() const { return Kind == Struct; }
  bool IsIntegerType() const {
    switch (Ty) {
    case Char:
    case UnsignedChar:
    case Int:
    case UnsignedInt:
      return true;
    default:
      return false;
    }
  }

  bool IsConst() const { return Qualifiers & Const; }

  std::vector<Type> &GetTypeList() { return TypeList; }
  std::vector<Type> &GetParameterList() { return ParameterList; }
  VariantKind GetReturnType() const { return Ty; }

  std::vector<unsigned> &GetDimensions() {
    assert(IsArray() && "Must be an Array type to access Dimensions.");
    return Dimensions;
  }

   void SetDimensions(std::vector<unsigned> D) {
     Kind = Array;
     Dimensions = std::move(D);
   }

  std::vector<Type> &GetArgTypes() { return ParameterList; }

  Type GetStructMemberType(const std::string &Member) {
    for (auto &T : TypeList)
      if (T.GetName() == Member)
        return T;

    return Type(VariantKind::Invalid);
  }

  friend bool operator==(const Type &lhs, const Type &rhs) {
    bool result = lhs.Kind == rhs.Kind && lhs.Ty == rhs.Ty;

    if (!result)
      return false;

    switch (lhs.Kind) {
    case Array:
      result = result && (lhs.Dimensions == rhs.Dimensions);
      break;
    default:
      break;
    }
    return result;
  }

  std::string ToString() const {
    if (IsFunction()) {
      auto TyStr = Type::ToString(this);
      auto ArgSize = ParameterList.size();
      if (ArgSize > 0)
        TyStr += " (";
      for (size_t i = 0; i < ArgSize; i++) {
        TyStr += Type::ToString(&ParameterList[i]);
        if (i + 1 < ArgSize)
          TyStr += ",";
        else
          TyStr += ")";
      }
      return TyStr;
    } else if (Kind == Array) {
      auto TyStr = Type::ToString(this);

      for (size_t i = 0; i < Dimensions.size(); i++)
        TyStr += "[" + std::to_string(Dimensions[i]) + "]";
      return TyStr;
    } else {
      return Type::ToString(this);
    }
  }

private:
  std::string Name; // For structs
  VariantKind Ty;
  uint8_t PointerLevel = 0;

  TypeKind Kind;
  unsigned Qualifiers = None;
  // TODO: revisit the use of union, deleted from here since
  // it just made things complicated
  std::vector<Type> TypeList;
  std::vector<Type> ParameterList;
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

  int GetIntVal() {
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