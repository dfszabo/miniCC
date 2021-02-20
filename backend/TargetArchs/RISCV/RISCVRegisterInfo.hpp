#ifndef RISCV_REGISTER_INFO_HPP
#define RISCV_REGISTER_INFO_HPP

#include "../../RegisterInfo.hpp"
#include "../../TargetRegister.hpp"

namespace RISCV {

enum Registers : unsigned {
  INVALID,
#define RISCV_REGISTER(ID, WIDTH, NAME, ALIAS) ID,
#include "RISCVRegisters.def"
};

class RISCVRegisterInfo : public RegisterInfo {
public:
  RISCVRegisterInfo();
  ~RISCVRegisterInfo() override {}

  TargetRegister *GetRegister(unsigned i) override;
  TargetRegister *GetRegisterByID(unsigned i) override;
  unsigned GetFrameRegister() override;
  unsigned GetStackRegister() override;

private:
  TargetRegister Registers[32];
};

} // namespace RISCV

#endif