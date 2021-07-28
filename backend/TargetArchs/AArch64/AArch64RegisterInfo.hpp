#ifndef AARCH64_REGISTER_INFO_HPP
#define AARCH64_REGISTER_INFO_HPP

#include "../../RegisterInfo.hpp"
#include "../../TargetRegister.hpp"

namespace AArch64 {

enum Registers : unsigned {
  INVALID,
#define AARCH64_REGISTER(ID, WIDTH, NAME, ALIAS) ID,
#include "AArch64Registers.def"
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
  unsigned GetZeroRegister() override;

private:
  TargetRegister Registers[67];
};

} // namespace AArch64

#endif