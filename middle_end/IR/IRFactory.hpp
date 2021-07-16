#ifndef IRFACTORY_HPP
#define IRFACTORY_HPP

#include "BasicBlock.hpp"
#include "Function.hpp"
#include "Instructions.hpp"
#include "Module.hpp"
#include "Value.hpp"
#include "../../backend/TargetMachine.hpp"
#include <map>
#include <memory>

class IRFactory {
private:
  Instruction *CreateBinaryInstruction(Instruction::IKind K, Value *L,
                                             Value *R) {
    auto Inst = std::make_unique<BinaryInstruction>(K, L, R, GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  BasicBlock *GetCurrentBB() { return CurrentModule.CurrentBB(); }

  Instruction *Insert(std::unique_ptr<Instruction> I) {
    return this->GetCurrentBB()->Insert(std::move(I));
  }

public:
  IRFactory() = delete;
  IRFactory(Module &M) : CurrentModule(M), ID(0) {}
  IRFactory(Module &M, TargetMachine *T) : TM(T), CurrentModule(M), ID(0) {}

  Instruction *CreateAND(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::AND, LHS, RHS);
  }

  Instruction *CreateOR(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::OR, LHS, RHS);
  }

  Instruction *CreateLSL(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::LSL, LHS, RHS);
  }

  Instruction *CreateLSR(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::LSR, LHS, RHS);
  }

  Instruction *CreateADD(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::ADD, LHS, RHS);
  }

  Instruction *CreateSUB(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::SUB, LHS, RHS);
  }

  Instruction *CreateMUL(Value *LHS, Value *RHS) {
    if (LHS->IsConstant() && RHS->IsConstant()) {
      uint64_t Val =
          ((Constant*)LHS)->GetIntValue() * ((Constant*)RHS)->GetIntValue();
      return CreateMOV(GetConstant(Val));
    }
    return CreateBinaryInstruction(Instruction::MUL, LHS, RHS);
  }

  Instruction *CreateDIV(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::DIV, LHS, RHS);
  }

  Instruction *CreateDIVU(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::DIVU, LHS, RHS);
  }

  Instruction *CreateMOD(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::MOD, LHS, RHS);
  }

  Instruction *CreateMODU(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::MODU, LHS, RHS);
  }

  // FIXME: revisit this, it may be better to make a unique instruction variant
  // for 'mov' instead of using UnaryInstruction
  UnaryInstruction *CreateMOV(Value *Operand, uint8_t BitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(
        Instruction::MOV, IRType::CreateInt(BitWidth), Operand,
        GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateSEXT(Value *Operand, uint8_t BitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(
        Instruction::SEXT, IRType::CreateInt(BitWidth), Operand,
        GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateZEXT(Value *Operand, uint8_t BitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(
        Instruction::ZEXT, IRType::CreateInt(BitWidth), Operand,
        GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateTRUNC(Value *Operand, uint8_t BitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(
        Instruction::TRUNC, IRType::CreateInt(BitWidth), Operand,
        GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateFTOI(Value *Operand, uint8_t FloatBitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(
        Instruction::FTOI, IRType::CreateFloat(FloatBitWidth), Operand,
        GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateITOF(Value *Operand, uint8_t IntBitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(
        Instruction::ITOF, IRType::CreateInt(IntBitWidth), Operand,
        GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  CallInstruction *CreateCALL(std::string &Name, std::vector<Value *> Args,
                              IRType Type) {
    auto Inst =
        std::make_unique<CallInstruction>(Name, Args, Type, GetCurrentBB());
    auto InstPtr = Inst.get();

    if (!Type.IsVoid())
      Inst->SetID(ID++);

    Insert(std::move(Inst));

    return InstPtr;
  }

  ReturnInstruction *CreateRET(Value *ReturnValue) {
    auto Inst =
        std::make_unique<ReturnInstruction>(ReturnValue, GetCurrentBB());
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  StackAllocationInstruction *CreateSA(std::string Identifier, IRType Type) {
    auto Inst = std::make_unique<StackAllocationInstruction>(
        Identifier, Type, CurrentModule.GetBB(0));
    auto InstPtr = Inst.get();
    Inst->SetID(ID++);
    CurrentModule.GetBB(0)->InsertSA(std::move(Inst));

    return InstPtr;
  }

  GetElementPointerInstruction *CreateGEP(IRType ResultType, Value *Source,
                            Value* Index) {
    auto Inst = std::make_unique<GetElementPointerInstruction>(ResultType, Source, Index,
                                                  GetCurrentBB());
    auto InstPtr = Inst.get();
    Inst->SetID(ID++);
    Insert(std::move(Inst));

    return InstPtr;
  }

  StoreInstruction *CreateSTR(Value *Source, Value *Destination) {
    auto Inst =
        std::make_unique<StoreInstruction>(Source, Destination, GetCurrentBB());
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  LoadInstruction *CreateLD(IRType ResultType, Value *Source,
                            Value *Offset = nullptr) {
    auto Inst = std::make_unique<LoadInstruction>(ResultType, Source, Offset,
                                                  GetCurrentBB());
    auto InstPtr = Inst.get();
    Inst->SetID(ID++);
    Insert(std::move(Inst));

    return InstPtr;
  }

  MemoryCopyInstruction *CreateMEMCOPY(Value *Destination, Value *Source, size_t Bytes) {
    auto Inst = std::make_unique<MemoryCopyInstruction>(Destination, Source, Bytes,
                                                  GetCurrentBB());
    auto InstPtr = Inst.get();
    Inst->SetID(ID++);
    Insert(std::move(Inst));

    return InstPtr;
  }

  CompareInstruction *CreateCMP(CompareInstruction::CompRel Relation,
                                Value *LHS, Value *RHS) {
    auto Inst = std::make_unique<CompareInstruction>(LHS, RHS, Relation,
                                                     GetCurrentBB());
    auto InstPtr = Inst.get();
    Inst->SetID(ID++);
    Insert(std::move(Inst));

    return InstPtr;
  }

  JumpInstruction *CreateJUMP(BasicBlock *Destination) {
    auto Inst = std::make_unique<JumpInstruction>(
        JumpInstruction(Destination, GetCurrentBB()));
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  BranchInstruction *CreateBR(Value *Condition, BasicBlock *True,
                              BasicBlock *False = nullptr) {
    auto Inst = std::make_unique<BranchInstruction>(
        BranchInstruction(Condition, True, False, GetCurrentBB()));
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  GlobalVariable *CreateGlobalVar(std::string &Identifier, IRType Type) {
    auto GlobalVar = new GlobalVariable(Identifier, Type);
    GlobalVar->SetID(ID++);

    return GlobalVar;
  }

  GlobalVariable *CreateGlobalVar(std::string &Identifier, IRType Type,
                                  std::vector<uint64_t> InitList) {
    auto GlobalVar = new GlobalVariable(Identifier, Type, std::move(InitList));
    GlobalVar->SetID(ID++);

    return GlobalVar;
  }

  void CreateNewFunction(std::string &Name, IRType ReturnType) {
    CurrentModule.AddFunction(std::move(Function(Name, ReturnType)));
    SymbolTable.clear();
    LabelTable.clear();
    ID = 0;
  }

  void AddGlobalVariable(Value *GlobalValue) {
    std::unique_ptr<Value> GV(GlobalValue);
    CurrentModule.AddGlobalVar(std::move(GV));
    SetGlobalScope(false);
  }

  Value *GetGlobalVar(const std::string &Identifier) {
    return CurrentModule.GetGlobalVar(Identifier);
  }

  bool IsGlobalValue(Value *Value) const {
    return CurrentModule.IsGlobalValue(Value);
  }

  bool IsGlobalScope() const { return GlobalScope; }
  void SetGlobalScope(const bool v = true) { GlobalScope = v; }

  BasicBlock *CreateBasicBlock() { return CurrentModule.CreateBasicBlock(); }

  Function *GetCurrentFunction() { return CurrentModule.CurrentFunction(); }

  void InsertBB(std::unique_ptr<BasicBlock> BB) {
    // Modify label name to guarantee its uniqueness
    auto NewName = BB->GetName() + std::to_string(LabelTable[BB->GetName()]);
    LabelTable[BB->GetName()]++;
    BB->SetName(NewName);

    GetCurrentFunction()->Insert(std::move(BB));
  }

  void Insert(std::unique_ptr<FunctionParameter> FP) {
    FP->SetID(ID++);
    GetCurrentFunction()->Insert(std::move(FP));
  }

  void AddToSymbolTable(std::string &Identifier, Value *Value) {
    SymbolTable[Identifier] = Value;
  }

  Value *GetSymbolValue(const std::string &Identifier) {
    return SymbolTable[Identifier];
  }

  Constant *GetConstant(uint64_t C) {
    if (auto ConstVal = IntConstantPool[C].get(); ConstVal != nullptr)
      return ConstVal;

    IntConstantPool[C] = std::make_unique<Constant>(C);
    return IntConstantPool[C].get();
  }

  Constant *GetConstant(double C) {
    if (auto ConstVal = FPConstantPool[C].get(); ConstVal != nullptr)
      return ConstVal;

    FPConstantPool[C] = std::make_unique<Constant>(C);
    return FPConstantPool[C].get();
  }

  std::vector<BasicBlock*> &GetLoopIncrementBBsTable() {
    return LoopIncrementBBsTable;
  }

  std::vector<BasicBlock*> &GetBreaksEndBBsTable() {
    return BreaksTargetBBsTable;
  }

  TargetMachine *GetTargetMachine() { return TM; }

private:
  TargetMachine *TM;

  Module &CurrentModule;

  /// A counter essentially, which used to give Values a uniq ID.
  unsigned ID;

  /// Shows whether we are in the global scope or not.
  bool GlobalScope = false;

  /// To store already created integer constants
  std::map<uint64_t, std::unique_ptr<Constant>> IntConstantPool;

  /// To store already created floating point constants
  std::map<double, std::unique_ptr<Constant>> FPConstantPool;

  // FIXME: Consider putting these to Function class

  /// Hold the local symbols for the current function.
  std::map<std::string, Value *> SymbolTable;

  /// To keep track how many times each label were defined. This number
  /// can be used to concatenate it to the label to make it unique.
  std::map<std::string, unsigned> LabelTable;

  /// For context information for "continue" statements. Containing the pointer
  /// to the basic block which will be the target of the generated jump.
  std::vector<BasicBlock*> LoopIncrementBBsTable;

  /// For context information for "break" statements. Containing the pointer
  /// to the basic block which will be the target of the generated jump.
  std::vector<BasicBlock*> BreaksTargetBBsTable;
};

#endif
