#ifndef MACHINE_OPERAND_HPP
#define MACHINE_OPERAND_HPP

#include "LowLevelType.hpp"
#include "TargetRegister.hpp"
#include <cstdint>
#include <iostream>

class TargetMachine;

class MachineOperand {
public:
  enum MOKind : unsigned {
    NONE,
    REGISTER,
    VIRTUAL_REGISTER,
    INT_IMMEDIATE,
    MEMORY_ADDRESS,
    STACK_ACCESS,
    PARAMETER,
    LABEL,
    FUNCTION_NAME,
  };

  MachineOperand() {}

  void SetToVirtualRegister() { Type = VIRTUAL_REGISTER; }
  void SetToRegister() { Type = REGISTER; }
  void SetToIntImm() { Type = INT_IMMEDIATE; }
  void SetToMemAddr() { Type = MEMORY_ADDRESS; }
  void SetToStackAccess() { Type = STACK_ACCESS; }
  void SetToParameter() { Type = PARAMETER; }
  void SetToLabel() { Type = LABEL; }
  void SetToFunctionName() { Type = FUNCTION_NAME; }

  int64_t GetImmediate() const { return Value; }
  int64_t GetReg() const { return Value; }
  uint64_t GetSlot() const { return Value; }
  void SetReg(uint64_t V) { SetValue(V); }
  void SetValue(uint64_t V) { Value = V; }

  void SetOffset(int o) { Offset = o; }
  int GetOffset() const { return Offset; }

  void SetType(LowLevelType LLT) { this->LLT = LLT; }
  LowLevelType GetType() const { return LLT; }

  const char *GetLabel() { return Label; }
  const char *GetFunctionName() { return Label; }
  void SetLabel(const char *L) { Label = L; }

  bool IsRegister() const { return Type == REGISTER; }
  bool IsVirtualReg() const { return Type == VIRTUAL_REGISTER; }
  bool IsImmediate() const { return Type == INT_IMMEDIATE; }
  bool IsMemory() const { return Type == MEMORY_ADDRESS; }
  bool IsStackAccess() const { return Type == STACK_ACCESS; }
  bool IsParameter() const { return Type == PARAMETER; }
  bool IsLabel() const { return Type == LABEL; }
  bool IsFunctionName() const { return Type == FUNCTION_NAME; }

  unsigned GetSize() { return LLT.GetBitWidth(); }

  static MachineOperand CreateRegister(uint64_t Reg, unsigned BitWidth = 32) {
    MachineOperand MO;
    MO.SetToRegister();
    MO.SetReg(Reg);
    MO.SetType(LowLevelType::CreateINT(BitWidth));
    return MO;
  }

  static MachineOperand CreateVirtualRegister(uint64_t Reg, unsigned BitWidth = 32) {
    MachineOperand MO;
    MO.SetToVirtualRegister();
    MO.SetReg(Reg);
    MO.SetType(LowLevelType::CreateINT(BitWidth));
    return MO;
  }

  static MachineOperand CreateImmediate(uint64_t Val, unsigned BitWidth = 32) {
    MachineOperand MO;
    MO.SetToIntImm();
    MO.SetValue(Val);
    MO.SetType(LowLevelType::CreateINT(BitWidth));
    return MO;
  }

  static MachineOperand CreateMemory(uint64_t Id) {
    MachineOperand MO;
    MO.SetToMemAddr();
    MO.SetValue(Id);
    return MO;
  }

  static MachineOperand CreateStackAccess(uint64_t Slot, int Offset = 0) {
    MachineOperand MO;
    MO.SetToStackAccess();
    MO.SetOffset(Offset);
    MO.SetValue(Slot);
    return MO;
  }

  static MachineOperand CreateParameter(uint64_t Val) {
    MachineOperand MO;
    MO.SetToParameter();
    MO.SetReg(Val);
    return MO;
  }

  static MachineOperand CreateLabel(const char* Label) {
    MachineOperand MO;
    MO.SetToLabel();
    MO.SetLabel(Label);
    return MO;
  }

  static MachineOperand CreateFunctionName(const char* Label) {
    MachineOperand MO;
    MO.SetToFunctionName();
    MO.SetLabel(Label);
    return MO;
  }

  void Print(TargetMachine *TM) const;

private:
  unsigned Type = NONE;
  uint64_t Value = ~0;
  int Offset = 0;
  LowLevelType LLT;
  const char *Label = nullptr;
};

#endif
