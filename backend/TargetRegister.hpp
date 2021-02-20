#ifndef TARGET_REGISTER_HPP
#define TARGET_REGISTER_HPP

#include <set>
#include <string>

class TargetRegister {
public:
  TargetRegister() {}

  void SetID(unsigned ID) { this->ID = ID; }
  unsigned GetID() { return ID; }

  void SetBitWidth(unsigned BitWidth) { this->BitWidth = BitWidth; }

  void SetName(std::string &N) { Name = N; }
  void SetName(const char *N) { Name = N; }
  std::string &GetName() { return Name; }

  void SetAlias(const char *N) { AliasName = N; }
  std::string &GetAlias() { return AliasName; }

  static TargetRegister Create(unsigned ID, unsigned BitWidth, const char *Name,
                               const char *Alias) {
    TargetRegister NewReg;
    NewReg.SetID(ID);
    NewReg.SetBitWidth(BitWidth);
    NewReg.SetName(Name);
    NewReg.SetAlias(Alias);
    return NewReg;
  }

private:
  unsigned ID = 0;
  unsigned BitWidth = 0;
  std::string Name;
  std::string AliasName;
};

class RegisterClass {
public:
  void AddRegister(unsigned ID) { Registers.insert(ID); }

  bool Contains(unsigned ID) { return 0 != Registers.count(ID); }

private:
  std::set<unsigned, std::greater<unsigned>> Registers;
};

#endif
