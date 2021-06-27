#ifndef TARGET_MACHINE_HPP
#define TARGET_MACHINE_HPP

#include "InstructionDefinitions.hpp"
#include "MachineInstruction.hpp"
#include "RegisterInfo.hpp"
#include "TargetABI.hpp"
#include "TargetInstructionLegalizer.hpp"
#include <memory>

class TargetMachine {
public:
  TargetMachine() {}
  TargetMachine(InstructionDefinitions *ID) : InstrDefs(ID) {}
  virtual ~TargetMachine() {}

  TargetABI *GetABI() { return ABI.get(); }
  InstructionDefinitions *GetInstrDefs() { return InstrDefs.get(); }
  RegisterInfo *GetRegInfo() { return RegInfo.get(); }
  TargetInstructionLegalizer *GetLegalizer() { return Legalizer.get(); }

  virtual uint8_t GetPointerSize() {
    return 32; // default
  }

  bool SelectInstruction(MachineInstruction *MI);

  virtual bool SelectADD(MachineInstruction *MI) { return false; }
  virtual bool SelectSUB(MachineInstruction *MI) { return false; }
  virtual bool SelectMUL(MachineInstruction *MI) { return false; }
  virtual bool SelectDIV(MachineInstruction *MI) { return false; }
  virtual bool SelectMOD(MachineInstruction *MI) { return false; }
  virtual bool SelectCMP(MachineInstruction *MI) { return false; }
  virtual bool SelectSEXT(MachineInstruction *MI) { return false; }
  virtual bool SelectZEXT(MachineInstruction *MI) { return false; }
  virtual bool SelectTRUNC(MachineInstruction *MI) { return false; }
  virtual bool SelectZEXT_LOAD(MachineInstruction *MI) { return false; }
  virtual bool SelectLOAD(MachineInstruction *MI) { return false; }
  virtual bool SelectLOAD_IMM(MachineInstruction *MI) { return false; }
  virtual bool SelectMOV(MachineInstruction *MI) { return false; }
  virtual bool SelectSTORE(MachineInstruction *MI) { return false; }
  virtual bool SelectSTACK_ADDRESS(MachineInstruction *MI) { return false; }
  virtual bool SelectGLOBAL_ADDRESS(MachineInstruction *MI) { return false; }
  virtual bool SelectBRANCH(MachineInstruction *MI) { return false; }
  virtual bool SelectJUMP(MachineInstruction *MI) { return false; }
  virtual bool SelectCALL(MachineInstruction *MI) { return false; }
  virtual bool SelectRET(MachineInstruction *MI) { return false; }

protected:
  std::unique_ptr<TargetABI> ABI = nullptr;
  std::unique_ptr<InstructionDefinitions> InstrDefs = nullptr;
  std::unique_ptr<RegisterInfo> RegInfo = nullptr;
  std::unique_ptr<TargetInstructionLegalizer> Legalizer = nullptr;
};

#endif
