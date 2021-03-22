#ifndef MACHINEINSTRUCTION_HPP
#define MACHINEINSTRUCTION_HPP

#include "MachineOperand.hpp"
#include <vector>

class MachineBasicBlock;

class MachineInstruction {
  using OperandList = std::vector<MachineOperand>;

public:
  enum OperationCode : unsigned {
    // Arithmetic and Logical
    AND = 1 << 16,
    OR,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    CMP, // Logical comparison

    // Conversions
    FTOI, // Float TO Integer
    ITOF, // Integer TO Float

    // Control flow operations
    CALL,
    JUMP,
    BRANCH,
    RET,

    // Memory operations
    LOAD,
    STORE,
    STACK_ALLOC,
  };

  enum CMPRelation { INVALID, EQ, NE, LT, GT, LE, GE };

  enum Flags : unsigned {
    IS_LOAD = 1,
    IS_STORE = 1 << 1,
  };

  MachineInstruction() {}
  MachineInstruction(unsigned Opcode, MachineBasicBlock *Parent)
      : Opcode(Opcode), Parent(Parent) {}

  unsigned GetOpcode() const { return Opcode; }
  void SetOpcode(unsigned Opcode) { this->Opcode = Opcode; }

  size_t GetOperandsNumber() const { return Operands.size(); }

  MachineOperand *GetOperand(size_t Index) { return &Operands[Index]; }
  OperandList &GetOperands() { return Operands; }

  void AddOperand(MachineOperand MO) { Operands.push_back(MO); }
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
      if (Operands[i].IsStackAccess())
        Operands.erase(Operands.begin() + i--);
  }

  void AddRegister(uint64_t Reg) {
    AddOperand(MachineOperand::CreateRegister(Reg));
  }

  void AddImmediate(uint64_t Num) {
    AddOperand(MachineOperand::CreateImmediate(Num));
  }

  void AddMemory(uint64_t Id) {
    AddOperand(MachineOperand::CreateMemory(Id));
  }

  void AddStackAccess(uint64_t Slot) {
    AddOperand(MachineOperand::CreateStackAccess(Slot));
  }

  void AddLabel(const char* Label) {
    AddOperand(MachineOperand::CreateLabel(Label));
  }

  void AddAttribute(unsigned AttributeFlag) {
    OtherAttributes |= AttributeFlag;
  }

  bool IsFallThroughBranch() const { return Operands.size() == 2; }
  bool IsLoad() const { return Opcode == LOAD || (OtherAttributes & IS_LOAD); }
  bool IsStore() const { return Opcode == STORE || (OtherAttributes & IS_STORE); }
  bool IsLoadOrStore() const { return IsLoad() || IsStore(); }

private:
  unsigned Opcode = 0;

  // Capture things like the relation for compare instructions
  unsigned Attributes = 0;
  unsigned OtherAttributes = 0;
  OperandList Operands;

  MachineBasicBlock *Parent = nullptr;
};

#endif
