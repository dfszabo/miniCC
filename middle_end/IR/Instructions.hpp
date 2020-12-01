#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "Value.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

class BasicBlock;

class Instruction : public Value {
public:
  enum IKind {
    // Arithmetic and Logical
    AND,
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

  static std::string AsString(IKind IK);

  Instruction(IKind K, BasicBlock *P, IRType V)
      : InstKind(K), Parent(P), Value(V) {}

  bool IsStackAllocation() const { return InstKind == STACK_ALLOC; }

  bool IsTerminator() { return BasicBlockTerminator; }

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
      : Instruction(UO, P, ResultType), Op(Operand) {}

  void Print() const override;

private:
  Value *Op;
};

class CompareInstruction : public Instruction {
public:
  enum CompRel { EQ, NE, LT, GT, LE, GE };

  CompareInstruction(Value *L, Value *R, CompRel REL, BasicBlock *P)
      : Instruction(Instruction::CMP, P, IRType(IRType::SINT, 1)), LHS(L),
        RHS(R), Relation(REL) {}

  const char *GetRelString() const;

  void InvertRelation();

  void Print() const override;

private:
  CompRel Relation;
  Value *LHS;
  Value *RHS;
};

class CallInstruction : public Instruction {
public:
  CallInstruction(const std::string &N, std::vector<Value *> &A, IRType T,
                  BasicBlock *P)
      : Instruction(Instruction::CALL, P, T), Name(N), Arguments(A) {}

  CallInstruction(const std::string N, IRType T, BasicBlock *P)
      : Instruction(Instruction::CALL, P, T), Name(N) {}

  std::string &GetName() { return Name; }
  std::vector<Value *> &GetArgs() { return Arguments; }

  void Print() const override;

private:
  std::string Name;
  std::vector<Value *> Arguments;
};

class JumpInstruction : public Instruction {
public:
  JumpInstruction(BasicBlock *D, BasicBlock *P)
      : Instruction(Instruction::JUMP, P, IRType(IRType::NONE)), Target(D) {}

  void SetTargetBB(BasicBlock *Target) { Target = Target; }

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

  void Print() const override;

private:
  Value *Condition;
  BasicBlock *TrueTarget;
  BasicBlock *FalseTarget;
};

class ReturnInstruction : public Instruction {
public:
  ReturnInstruction(Value *RV, BasicBlock *P)
      : Instruction(Instruction::RET, P, RV->GetType()), RetVal(RV) {
    BasicBlockTerminator = true;
  }

  void Print() const override;

private:
  Value *RetVal;
};

class StackAllocationInstruction : public Instruction {
public:
  StackAllocationInstruction(std::string &S, IRType T, BasicBlock *P)
      : Instruction(Instruction::STACK_ALLOC, P, T), VariableName(S) {}

  void Print() const override;

private:
  std::string VariableName;
};

class StoreInstruction : public Instruction {
public:
  StoreInstruction(Value *S, Value *D, BasicBlock *P)
      : Instruction(Instruction::STORE, P, IRType::NONE), Source(S),
        Destination(D) {}

  void Print() const override;

private:
  Value *Source;
  Value *Destination;
};

class LoadInstruction : public Instruction {
public:
  LoadInstruction(IRType T, Value *S, Value *O, BasicBlock *P)
      : Instruction(Instruction::LOAD, P, T), Source(S), Offset(O) {}

  LoadInstruction(IRType T, Value *S, BasicBlock *P)
      : Instruction(Instruction::LOAD, P, T), Source(S), Offset(nullptr) {}

  void Print() const override;

private:
  Value *Source;
  Value *Offset;
};

#endif
