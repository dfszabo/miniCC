#ifndef RISCV_REGISTER_INFO_HPP
#define RISCV_REGISTER_INFO_HPP

#include "../../RegisterInfo.hpp"
#include "../../TargetRegister.hpp"

namespace RISCV {

enum Registers : unsigned {
  INVALID,
#define RISCV_REGISTER(ID, WIDTH, NAME, ALIAS) ID,
#include "RISCVRegisters.def"
  REGISTERS_END,
};

class RISCVRegisterInfo : public RegisterInfo {
public:
  RISCVRegisterInfo();
  ~RISCVRegisterInfo() override {}

  TargetRegister *GetRegister(unsigned i) override;
  TargetRegister *GetRegisterByID(unsigned i) override;
  unsigned GetFrameRegister() override;
  unsigned GetLinkRegister() override;
  unsigned GetStackRegister() override;
  unsigned GetStructPtrRegister() override;
  unsigned GetZeroRegister(const unsigned BitWidth) override;
  TargetRegister *GetParentReg(unsigned ID) override {
    return nullptr;
  }
  unsigned GetRegisterClass(const unsigned BitWidth, const bool IsFP) override;
  virtual std::string GetRegClassString(const unsigned RegClass) override;
  unsigned GetRegClassFromReg(const unsigned Reg) override;
  unsigned GetRegClassRegsSize(const unsigned RegClass) override;

private:
  TargetRegister Registers[REGISTERS_END - 1];
  std::vector<std::string> RegClassEnumStrings;
};

} // namespace RISCV

#endif