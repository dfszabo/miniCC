#ifndef REGISTER_INFO_HPP
#define REGISTER_INFO_HPP

#include "TargetRegister.hpp"
#include <cassert>

class RegisterInfo {
public:
  RegisterInfo() {}
  virtual ~RegisterInfo() {}

  virtual unsigned GetFrameRegister() { return 0; }
  virtual unsigned GetLinkRegister() { return 0; }
  virtual unsigned GetStackRegister() { return 0; }
  virtual unsigned GetStructPtrRegister() { return ~0; }
  virtual unsigned GetZeroRegister(const unsigned BitWidth) { return ~0; }
  virtual TargetRegister *GetParentReg(unsigned ID) {
    assert(!"Unimplemented");
    return nullptr;
  }
  virtual TargetRegister *GetRegister(unsigned i) {
    assert(!"Unimplemented");
    return nullptr;
  }
  virtual TargetRegister *GetRegisterByID(unsigned i) {
    assert(!"Unimplemented");
    return nullptr;
  }

  virtual unsigned GetRegisterClass(const unsigned BitWidth, const bool IsFP) {
    assert(!"Unimplemented");
    return ~0;
  }

  virtual std::string GetRegClassString(const unsigned RegClass) {
    assert(!"Unimplemented");
    return "";
  }

  virtual unsigned GetRegClassFromReg(const unsigned Reg) {
    assert(!"Unimplemented");
    return ~0;
  }

  /// Return the max bit size of the registers from this class
  /// Example: AArch64 GPR32 -> 32, GPR -> 64
  virtual unsigned GetRegClassRegsSize(const unsigned RegClass) {
    assert(!"Unimplemented");
    return ~0;
  }
};

#endif
