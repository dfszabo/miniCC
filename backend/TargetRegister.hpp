#ifndef TARGET_REGISTER_HPP
#define TARGET_REGISTER_HPP

#include <set>
#include <string>
#include <vector>

class TargetRegister {
public:
  TargetRegister() {}

  void SetID(unsigned ID) { this->ID = ID; }
  unsigned GetID() { return ID; }

  void SetBitWidth(unsigned BitWidth) { this->BitWidth = BitWidth; }
  unsigned GetBitWidth() const { return BitWidth; }

  void SetName(std::string &N) { Name = N; }
  void SetName(const char *N) { Name = N; }
  std::string &GetName() { return Name; }

  void SetAlias(const char *N) { AliasName = N; }
  std::string &GetAlias() { return AliasName; }

  void SetSubRegs(std::vector<unsigned> &N) { SubRegisters = N; }
  std::vector<unsigned> &GetSubRegs() { return SubRegisters; }

  void SetFP(bool p) { FPCapable = p; }
  bool IsFP() const { return FPCapable; }

  static TargetRegister Create(unsigned ID, unsigned BitWidth, const char *Name,
                               const char *Alias,
                               std::vector<unsigned> SubRegs = {},
                               bool FP = false) {
    TargetRegister NewReg;
    NewReg.SetID(ID);
    NewReg.SetBitWidth(BitWidth);
    NewReg.SetName(Name);
    NewReg.SetAlias(Alias);
    NewReg.SetFP(FP);
    NewReg.SetSubRegs(SubRegs);
    return NewReg;
  }

private:
  unsigned ID = 0;
  unsigned BitWidth = 0;
  std::string Name;
  std::string AliasName;
  std::vector<unsigned> SubRegisters;
  bool FPCapable;
};

#endif
