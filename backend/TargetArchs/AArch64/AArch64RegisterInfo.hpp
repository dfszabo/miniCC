#ifndef AARCH64_REGISTER_INFO_HPP
#define AARCH64_REGISTER_INFO_HPP

#include "../../RegisterInfo.hpp"
#include "../../TargetRegister.hpp"

namespace AArch64 {

enum Registers : unsigned {
  INVALID,
#define AARCH64_REGISTER(ID, WIDTH, NAME, ALIAS) ID,
#include "AArch64Registers.def"
  REGISTERS_END,
};

class AArch64RegisterInfo : public RegisterInfo {
public:
  AArch64RegisterInfo();
  ~AArch64RegisterInfo() override {}

  TargetRegister *GetRegister(unsigned i) override;
  TargetRegister *GetRegisterByID(unsigned i) override;
  TargetRegister *GetParentReg(unsigned ID) override;
  unsigned GetFrameRegister() override;
  unsigned GetLinkRegister() override;
  unsigned GetStackRegister() override;
  unsigned GetStructPtrRegister() override;
  unsigned GetZeroRegister(const unsigned BitWidth) override;
  unsigned GetRegisterClass(const unsigned BitWidth, const bool IsFP) override;
  virtual std::string GetRegClassString(const unsigned RegClass) override;
  unsigned GetRegClassFromReg(const unsigned Reg) override;
  unsigned GetRegClassRegsSize(const unsigned RegClass) override;

private:
  TargetRegister Registers[REGISTERS_END - 1];
  std::vector<std::string> RegClassEnumStrings;
};

} // namespace AArch64

#endif