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
  virtual unsigned GetZeroRegister() { return ~0; }
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
};

#endif
