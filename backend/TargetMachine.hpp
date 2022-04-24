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

  virtual uint8_t GetPointerSize() { return ~0; }
  virtual uint8_t GetIntSize() { return ~0; }
  virtual uint8_t GetLongSize() { return ~0; }

  bool IsMemcpySupported() const { return ABI->HasCLib(); }

  bool SelectInstruction(MachineInstruction *MI);

  virtual bool SelectAND(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectOR(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectXOR(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectLSL(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectLSR(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectADD(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectADDS(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectADDC(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectSUB(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectMUL(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectMULHU(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectDIV(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectDIVU(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectMOD(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectMODU(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectCMP(MachineInstruction *MI) { assert(!"Unimplemented"); }
  
  virtual bool SelectCMPF(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectADDF(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectSUBF(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectMULF(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectDIVF(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectITOF(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectFTOI(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectMOVF(MachineInstruction *MI) { assert(!"Unimplemented"); }

  virtual bool SelectSEXT(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectZEXT(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectTRUNC(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectZEXT_LOAD(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectLOAD(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectLOAD_IMM(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectMOV(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectSTORE(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectSTACK_ADDRESS(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectGLOBAL_ADDRESS(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectBRANCH(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectJUMP(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectCALL(MachineInstruction *MI) { assert(!"Unimplemented"); }
  virtual bool SelectRET(MachineInstruction *MI) { assert(!"Unimplemented"); }

protected:
  std::unique_ptr<TargetABI> ABI = nullptr;
  std::unique_ptr<InstructionDefinitions> InstrDefs = nullptr;
  std::unique_ptr<RegisterInfo> RegInfo = nullptr;
  std::unique_ptr<TargetInstructionLegalizer> Legalizer = nullptr;
};

#endif
