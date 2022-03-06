#ifndef MACHINEINSTRUCTION_HPP
#define MACHINEINSTRUCTION_HPP

#include "MachineOperand.hpp"
#include <cassert>
#include <iostream>
#include <vector>

class MachineBasicBlock;
class TargetMachine;

class MachineInstruction {
  using OperandList = std::vector<MachineOperand>;

public:
  // LLIR operations
  enum OperationCode : unsigned {
    // Integer Arithmetic and Logical
    AND = 1 << 16,
    OR,
    XOR,
    LSL,
    LSR,
    ADD,
    SUB,
    MUL,
    DIV,
    DIVU,
    MOD,
    MODU,
    CMP, // Logical comparison

    // Floating point Arithmetic and Logical
    ADDF,
    SUBF,
    MULF,
    DIVF,
    CMPF,

    // Conversions
    SEXT, // Sign extension
    ZEXT, // Zero extension
    TRUNC, // Truncating
    FTOI, // Float TO Integer
    ITOF, // Integer TO Float
    BITCAST,

    // Control flow operations
    CALL,
    JUMP,
    BRANCH,
    RET,

    // Moves and constant materializations
    LOAD_IMM,
    MOV,
    MOVF,

    // Memory operations
    LOAD,
    STORE,
    STACK_ALLOC,
    STACK_ADDRESS,
    GLOBAL_ADDRESS,

    // combined load and sign/zero extension
    SEXT_LOAD,
    ZEXT_LOAD,

    INVALID_OP,
  };

  enum CMPRelation { INVALID, EQ, NE, LT, GT, LE, GE };

  enum Flags : unsigned {
    IS_LOAD = 1,
    IS_STORE = 1 << 1,
    IS_EXPANDED = 1 << 2,
    IS_RETURN = 1 << 3,
  };

  MachineInstruction() {}
  MachineInstruction(unsigned Opcode, MachineBasicBlock *Parent)
      : Opcode(Opcode), Parent(Parent) {
    UpdateAttributes();
  }

  /// To update the instruction attributes based on the operation code.
  void UpdateAttributes();

  unsigned GetOpcode() const { return Opcode; }

  void SetOpcode(unsigned Opcode) {
    this->Opcode = Opcode;
    // only update attributes if this is a LLIR opcode
    if (Opcode >= (1 << 16)) {
      OtherAttributes = 0;
      UpdateAttributes();
    }
  }

  size_t GetOperandsNumber() const { return Operands.size(); }

  MachineOperand *GetOperand(size_t Index) {
    assert(Index < Operands.size());
    return &Operands[Index];
  }
  OperandList &GetOperands() { return Operands; }

  void AddOperand(MachineOperand MO) { Operands.push_back(MO); }
  void ReplaceOperand(MachineOperand MO, size_t index) {
    assert(index < Operands.size());
    Operands[index] = MO;
  }
  void SetAttributes(unsigned A) { Attributes = A; }

  unsigned GetRelation() const { return Attributes; }

  MachineBasicBlock *GetParent() const { return Parent; }
  void SetParent(MachineBasicBlock *BB) { Parent = BB; }

  void RemoveOperand(unsigned Index) {
    Operands.erase(Operands.begin() + Index);
  }

  void InsertOperand(unsigned Index, MachineOperand Operand) {
    Operands.insert(Operands.begin() + Index, Operand);
  }

  void RemoveMemOperand() {
    for (size_t i = 0; i < Operands.size(); i++)
      if (Operands[i].IsStackAccess() || Operands[i].IsMemory()) {
        Operands.erase(Operands.begin() + i--);
        return;
      }
    assert(!"Nothing was removed");
  }

  void AddVirtualRegister(uint64_t Reg, unsigned BitWidth = 32) {
    auto I = MachineOperand::CreateVirtualRegister(Reg);
    I.SetType(LowLevelType::CreateScalar(BitWidth));
    AddOperand(I);
  }

  void AddRegister(uint64_t Reg, unsigned BitWidth = 32) {
    auto I = MachineOperand::CreateRegister(Reg);
    I.SetType(LowLevelType::CreateScalar(BitWidth));
    AddOperand(I);
  }

  void AddImmediate(uint64_t Num, unsigned BitWidth = 32) {
    auto I = MachineOperand::CreateImmediate(Num);
    I.SetType(LowLevelType::CreateScalar(BitWidth));
    AddOperand(I);
  }

  void AddMemory(uint64_t Id, unsigned BitWidth = 32) {
    AddOperand(MachineOperand::CreateMemory(Id, BitWidth));
  }

  void AddMemory(uint64_t Id, int Offset, unsigned BitWidth) {
    AddOperand(MachineOperand::CreateMemory(Id, Offset, BitWidth));
  }

  void AddStackAccess(uint64_t Slot, unsigned Offset = 0) {
    AddOperand(MachineOperand::CreateStackAccess(Slot, Offset));
  }

  void AddLabel(const char *Label) {
    AddOperand(MachineOperand::CreateLabel(Label));
  }

  void AddFunctionName(const char *Name) {
    AddOperand(MachineOperand::CreateFunctionName(Name));
  }

  void AddGlobalSymbol(std::string Symbol) {
    AddOperand(MachineOperand::CreateGlobalSymbol(Symbol));
  }

  void AddAttribute(unsigned AttributeFlag) {
    OtherAttributes |= AttributeFlag;
  }

  bool IsFallThroughBranch() const { return Operands.size() == 2; }
  bool IsLoad() const { return Opcode == LOAD || (OtherAttributes & IS_LOAD); }
  bool IsStore() const {
    return Opcode == STORE || (OtherAttributes & IS_STORE);
  }
  bool IsAlreadyExpanded() const { return OtherAttributes & IS_EXPANDED; }
  bool IsReturn() const { return OtherAttributes & IS_RETURN; }

  /// flag the MI as expanded, so the legalizer can ignore it
  void FlagAsExpanded() { OtherAttributes |= IS_EXPANDED; }
  bool IsLoadOrStore() const { return IsLoad() || IsStore(); }
  bool IsAlreadySelected() const { return Opcode < 65536; }
  bool IsInvalid() const { return  Opcode == INVALID_OP; }

  void Print(TargetMachine *TM) const;

private:
  unsigned Opcode = INVALID_OP;

  // Capture things like the relation for compare instructions
  unsigned Attributes = 0;
  unsigned OtherAttributes = 0;
  OperandList Operands;

  MachineBasicBlock *Parent = nullptr;
};

#endif
