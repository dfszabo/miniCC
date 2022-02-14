#ifndef GLOBAL_DATA_HPP
#define GLOBAL_DATA_HPP

#include <string>
#include <vector>
#include <cassert>
#include <iostream>

/// To represent allocatable data, such as global variables and automatically
/// created data used for initializing arrays and structs
class GlobalData {
public:
  enum Directives {
    NONE = -1,
    ZERO,
    BYTE,
    HALF_WORD,
    WORD,
    DOUBLE_WORD,
    STRING
  };
  using InfoVector = std::vector<std::pair<Directives, std::string>>;

  GlobalData() {}
  GlobalData(const std::string &Name, const size_t Size)
      : Name(Name), Size(Size) {}

  std::string &GetName() { return Name; }
  void SetName(std::string &N) { Name = N; }

  size_t GetSize() const { return Size; }
  void SetSize(size_t S) { Size = S; }

  InfoVector &GetInitValues() { return InitValues; };

  void InsertAllocation(size_t ByteSize, int64_t InitVal) {
    Directives D = NONE;
    switch (ByteSize) {
    case 1:
      D = BYTE;
      break;
    case 2:
      D = HALF_WORD;
      break;
    case 4:
      D = WORD;
      break;
    case 8:
      D = DOUBLE_WORD;
      break;
    default:
      if (InitVal == 0) {
        D = ZERO;
        InitVal = Size;
      } else
       assert(!"Invalid size");
    }
    InitValues.push_back({D, std::to_string(InitVal)});
  }

  void InsertAllocation(std::string str, Directives d = STRING) {
    InitValues.push_back({d, str});
  }

  static std::string DirectiveToString(Directives D) {
    switch (D) {
    case NONE:
      return "";
    case ZERO:
      return "zero";
    case BYTE:
      return "byte";
    case HALF_WORD:
      return "short";
    case WORD:
      return "long";
    case DOUBLE_WORD:
      return "quad";
    case STRING:
      return "asciz";
    default:
      assert(!"Unreachable");
    }
  }

  void Print() const {
    std::string Str = Name + ":\n";
    for (auto &[Directive, InitVal] : InitValues) {
      Str += "  ." + DirectiveToString(Directive) + "\t";
      if (Directive != STRING)
        Str += InitVal + "\n";
      else
        Str += "\"" + InitVal + "\"\n";
    }
    std::cout << Str << std::endl;
  }

private:
  /// Name of the global object, which used to create its label
  std::string Name;

  /// How much bytes needs to be allocated for it overall. With the below
  /// example it would be 4.
  size_t Size;

  /// If it was initialised, then this vector hold the information about
  /// what directive needs to be used and with what value. Example:
  ///   struct S { int16_t, int8_t }
  ///   void foo() { struct S s = { 6, 9 }; }
  ///
  /// Will create the following private data:
  ///   .L_foo().s:
  ///     .short 6
  ///     .byte 9
  ///     .zero 1
  ///
  /// Assuming ".L_foo().s" will be the name of the automatically generated
  /// initializer data. ".zero 1" allocate one byte set to zero. This is for
  /// padding in the example.
  InfoVector InitValues;
};

#endif // GLOBAL_DATA_HPP
