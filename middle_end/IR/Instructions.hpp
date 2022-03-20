#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "Value.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

class BasicBlock;

class Instruction : public Value {
public:
  enum IKind {
    // Integer Arithmetic and Logical
    AND,
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
    SEXT,  // Sign extension
    ZEXT,  // Zero extension
    TRUNC, // Truncating
    FTOI,  // Float TO Integer
    ITOF,  // Integer TO Float
    BITCAST,

    // Control flow operations
    CALL,
    JUMP,
    BRANCH,
    RET,

    // Memory operations
    LOAD = RET + 4,
    STORE,
    MEM_COPY,
    STACK_ALLOC,
    GET_ELEM_PTR,
  };

  IKind GetInstructionKind() { return InstKind; }

  static std::string AsString(IKind IK);

  Instruction(IKind K, BasicBlock *P, IRType V)
      : InstKind(K), Parent(P), Value(std::move(V)) {
    BasicBlockTerminator = (InstKind == RET || InstKind == JUMP);
  }

  bool IsStackAllocation() const { return InstKind == STACK_ALLOC; }

  bool IsTerminator() const { return BasicBlockTerminator; }
  bool IsReturn() const { return InstKind == RET; }
  bool IsStore() const { return InstKind == STORE; }
  bool IsLoad() const { return InstKind == LOAD; }
  bool IsCall() const { return InstKind == CALL; }
  bool IsJump() const { return InstKind == JUMP; }
  bool IsGEP() const { return InstKind == GET_ELEM_PTR; }

  /// Is this instruction define a value? For example JUMP is not.
  virtual bool IsDef() const { return true; }

  virtual Value *Get1stUse() { return nullptr; }
  virtual Value *Get2ndUse() { return nullptr; }

  virtual void Set1stUse(Value *v) {}
  virtual void Set2ndUse(Value *v) {}

  virtual void Print() const { assert(!"Cannot print base class"); }

protected:
  IKind InstKind;
  BasicBlock *Parent = nullptr;
  bool BasicBlockTerminator = false;
};

class BinaryInstruction : public Instruction {
public:
  BinaryInstruction(IKind BO, Value *L, Value *R, BasicBlock *P)
      : Instruction(BO, P, L->GetType()), LHS(L), RHS(R) {}

  Value *GetLHS() { return LHS; }
  Value *GetRHS() { return RHS; }

  Value *Get1stUse() override { return LHS; }
  Value *Get2ndUse() override { return RHS; }

  void Set1stUse(Value *v) override { LHS = v; }
  void Set2ndUse(Value *v) override { RHS = v; }

  void Print() const override;

private:
  Value *LHS;
  Value *RHS;
};

class UnaryInstruction : public Instruction {
public:
  UnaryInstruction(IKind UO, Value *Operand, BasicBlock *P)
      : Instruction(UO, P, Operand->GetType()), Op(Operand) {}

  UnaryInstruction(IKind UO, IRType ResultType, Value *Operand, BasicBlock *P)
      : Instruction(UO, P, std::move(ResultType)), Op(Operand) {}

  Value *GetOperand() { return Op; }

  Value *Get1stUse() override { return Op; }

  void Set1stUse(Value *v) override { Op = v; }

  void Print() const override;

private:
  Value *Op;
};

class CompareInstruction : public Instruction {
public:
  enum CompRel : unsigned { INVALID, EQ, NE, LT, GT, LE, GE };

  CompareInstruction(Value *L, Value *R, CompRel REL, BasicBlock *P)
      : Instruction(L->IsFPType() && R->IsFPType() ? Instruction::CMPF
                                                   : Instruction::CMP,
                    P, IRType(IRType::SINT, 1)),
        LHS(L), RHS(R), Relation(REL) {}

  const char *GetRelString() const;

  Value *GetLHS() { return LHS; }
  Value *GetRHS() { return RHS; }
  unsigned GetRelation() { return (unsigned)Relation; }

  void InvertRelation();

  Value *Get1stUse() override { return LHS; }
  Value *Get2ndUse() override { return RHS; }

  void Set1stUse(Value *v) override { LHS = v; }
  void Set2ndUse(Value *v) override { RHS = v; }

  void Print() const override;

private:
  CompRel Relation = INVALID;
  Value *LHS;
  Value *RHS;
};

class CallInstruction : public Instruction {
public:
  CallInstruction(std::string N, std::vector<Value *> &A, IRType T,
                  BasicBlock *P, int StructIdx)
      : Instruction(Instruction::CALL, P, std::move(T)), Name(std::move(N)),
        Arguments(A), ImplicitStructArgIndex(StructIdx) {}

  CallInstruction(std::string N, IRType T, BasicBlock *P)
      : Instruction(Instruction::CALL, P, std::move(T)), Name(std::move(N)) {}

  std::string &GetName() { return Name; }
  std::vector<Value *> &GetArgs() { return Arguments; }
  int GetImplicitStructArgIndex() const { return ImplicitStructArgIndex; }

  bool IsDef() const override { return !GetType().IsVoid(); }

  void Print() const override;

private:
  std::string Name;
  std::vector<Value *> Arguments;
  int ImplicitStructArgIndex = -1;
};

class JumpInstruction : public Instruction {
public:
  JumpInstruction(BasicBlock *D, BasicBlock *P)
      : Instruction(Instruction::JUMP, P, IRType(IRType::NONE)), Target(D) {}

  BasicBlock *GetTargetBB() { return Target; }
  void SetTargetBB(BasicBlock *t) { Target = t; }

  std::string &GetTargetLabelName();

  bool IsDef() const override { return false; }

  void Print() const override;

private:
  BasicBlock *Target;
};

class BranchInstruction : public Instruction {
public:
  BranchInstruction(Value *C, BasicBlock *True, BasicBlock *False,
                    BasicBlock *P)
      : Instruction(Instruction::BRANCH, P, IRType(IRType::NONE)), Condition(C),
        TrueTarget(True), FalseTarget(False) {}

  Value *GetCondition() { return Condition; }
  std::string &GetTrueLabelName();
  std::string &GetFalseLabelName();

  bool HasFalseLabel() { return FalseTarget != nullptr; }
  bool IsDef() const override { return false; }

  Value *Get1stUse() override { return Condition; }

  void Set1stUse(Value *v) override { Condition = v; }

  void Print() const override;

private:
  Value *Condition;
  BasicBlock *TrueTarget;
  BasicBlock *FalseTarget;
};

class ReturnInstruction : public Instruction {
public:
  ReturnInstruction(Value *RV, BasicBlock *P)
      : Instruction(Instruction::RET, P,
                    RV ? RV->GetType() : IRType(IRType::NONE)),
        RetVal(RV) {
    BasicBlockTerminator = true;
  }

  Value *GetRetVal() const { return RetVal; }
  bool IsDef() const override { return false; }

  Value *Get1stUse() override { return RetVal; }

  void Set1stUse(Value *v) override { RetVal = v; }

  void Print() const override;

private:
  Value *RetVal;
};

class StackAllocationInstruction : public Instruction {
public:
  StackAllocationInstruction(std::string &S, IRType T, BasicBlock *P)
      : Instruction(Instruction::STACK_ALLOC, P, std::move(T)),
        VariableName(S) {
    this->GetTypeRef().SetPointerLevel(this->GetTypeRef().GetPointerLevel() +
                                       1);
  }

  void Print() const override;

private:
  std::string VariableName;
};

class GetElementPointerInstruction : public Instruction {
public:
  GetElementPointerInstruction(IRType T, Value *CompositeObject,
                               Value *AccessIndex, BasicBlock *P)
      : Instruction(Instruction::GET_ELEM_PTR, P, std::move(T)),
        Source(CompositeObject), Index(AccessIndex) {}

  Value *GetSource() const { return Source; }
  Value *GetIndex() { return Index; }

  Value *Get1stUse() override { return Source; }
  Value *Get2ndUse() override { return Index; }

  void Set1stUse(Value *v) override { Source = v; }
  void Set2ndUse(Value *v) override { Index = v; }

  void Print() const override;

private:
  Value *Source;
  Value *Index;
};

class StoreInstruction : public Instruction {
public:
  StoreInstruction(Value *S, Value *D, BasicBlock *P)
      : Instruction(Instruction::STORE, P, IRType(IRType::NONE)), Source(S),
        Destination(D) {
    assert(Source && Destination);
  }

  void Print() const override;

  Value *GetMemoryLocation() { return Destination; }
  Value *GetSavedValue() { return Source; }

  bool IsDef() const override { return false; }

  Value *Get1stUse() override { return Source; }
  Value *Get2ndUse() override { return Destination; }
  void Set1stUse(Value *v) override { Source = v; }
  void Set2ndUse(Value *v) override { Destination = v; }

private:
  Value *Source;
  Value *Destination;
};

class LoadInstruction : public Instruction {
public:
  LoadInstruction(IRType T, Value *S, Value *O, BasicBlock *P)
      : Instruction(Instruction::LOAD, P, std::move(T)), Source(S), Offset(O) {
    auto PtrLVL = this->GetTypeRef().GetPointerLevel();
    // Globals are handled differently, it is implicitly assumed that they
    // have 1 pointer level more, even though their IRType does not reflect this
    if (PtrLVL != 0 && !S->IsGlobalVar())
      PtrLVL--;
    this->GetTypeRef().SetPointerLevel(PtrLVL);
  }

  LoadInstruction(IRType T, Value *S, BasicBlock *P)
      : Instruction(Instruction::LOAD, P, std::move(T)), Source(S),
        Offset(nullptr) {
    auto PtrLVL = this->GetTypeRef().GetPointerLevel();
    if (PtrLVL != 0 && !S->IsGlobalVar())
      PtrLVL--;
    this->GetTypeRef().SetPointerLevel(PtrLVL);
  }

  void Print() const override;

  Value *GetMemoryLocation() { return Source; }

  Value *Get1stUse() override { return Source; }
  Value *Get2ndUse() override { return Offset; }

  void Set1stUse(Value *v) override { Source = v; }
  void Set2ndUse(Value *v) override { Offset = v; }

private:
  Value *Source;
  Value *Offset;
};

class MemoryCopyInstruction : public Instruction {
public:
  MemoryCopyInstruction(Value *Destination, Value *Source, size_t Bytes,
                        BasicBlock *P)
      : Instruction(Instruction::MEM_COPY, P, IRType()), Dest(Destination),
        Src(Source), N(Bytes) {}

  Value *GetDestination() { return Dest; }
  Value *GetSource() { return Src; }
  size_t GetSize() const { return N; }
  bool IsDef() const override { return false; }

  Value *Get1stUse() override { return Dest; }
  Value *Get2ndUse() override { return Src; }

  void Set1stUse(Value *v) override { Dest = v; }
  void Set2ndUse(Value *v) override { Src = v; }

  void Print() const override;

private:
  Value *Dest;
  Value *Src;
  size_t N;
};

#endif
