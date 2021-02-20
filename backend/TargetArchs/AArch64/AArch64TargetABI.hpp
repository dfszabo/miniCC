#ifndef AARCH64_TARGET_ABI_HPP
#define AARCH64_TARGET_ABI_HPP

#include "../../RegisterInfo.hpp"
#include "../../TargetABI.hpp"
#include "../../TargetRegister.hpp"

namespace AArch64 {

class AArch64TargetABI : public TargetABI {
public:
  AArch64TargetABI(RegisterInfo *RI);
};

} // namespace AArch64

#endif
