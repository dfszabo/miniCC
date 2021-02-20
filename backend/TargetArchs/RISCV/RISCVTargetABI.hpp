#ifndef RISCV_TARGET_ABI_HPP
#define RISCV_TARGET_ABI_HPP

#include "../../RegisterInfo.hpp"
#include "../../TargetABI.hpp"
#include "../../TargetRegister.hpp"

namespace RISCV {

class RISCVTargetABI : public TargetABI {
public:
  RISCVTargetABI(RegisterInfo *RI);
};

} // namespace RISCV

#endif
