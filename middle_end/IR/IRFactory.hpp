#ifndef IRFACTORY_HPP
#define IRFACTORY_HPP

#include "../../backend/TargetMachine.hpp"
#include "BasicBlock.hpp"
#include "Function.hpp"
#include "Instructions.hpp"
#include "Module.hpp"
#include "Value.hpp"
#include <map>
#include <memory>
#include <utility>

template <typename T>
static Value *EvaluateComparison(const T L, const T R,
                                 const CompareInstruction::CompRel Relation,
                                 Value *TrueVal, Value *FalseVal) {
  switch (Relation) {
  case CompareInstruction::EQ:
    return L == R ? TrueVal : FalseVal;
  case CompareInstruction::NE:
    return L != R ? TrueVal : FalseVal;
  case CompareInstruction::GT:
    return L > R ? TrueVal : FalseVal;
  case CompareInstruction::GE:
    return L >= R ? TrueVal : FalseVal;
  case CompareInstruction::LT:
    return L < R ? TrueVal : FalseVal;
  case CompareInstruction::LE:
    return L <= R ? TrueVal : FalseVal;
  default:
    assert(!"Unreachable");
  }
}

class IRFactory {
private:
  Value *
  EvaluateIntegerBinaryConstExpression(const int64_t L, const int64_t R,
                                       const uint8_t BW,
                                       const Instruction::IKind Operation) {
    switch (Operation) {
    case Instruction::ADD:
      return GetConstant((uint64_t)L + R, BW);
    case Instruction::SUB:
      return GetConstant((uint64_t)L - R, BW);
    case Instruction::MUL:
      return GetConstant((uint64_t)L * R, BW);
    case Instruction::DIV:
      assert(R != 0);
      return GetConstant((uint64_t)L / R, BW);
    case Instruction::DIVU:
      assert(R != 0);
      return GetConstant(((uint64_t)L) / ((uint64_t)R), BW);
    case Instruction::MOD:
      return GetConstant((uint64_t)L % R, BW);
    case Instruction::MODU:
      return GetConstant(((uint64_t)L) % ((uint64_t)R), BW);
    case Instruction::AND:
      return GetConstant(((uint64_t)L) & ((uint64_t)R), BW);
    case Instruction::OR:
      return GetConstant(((uint64_t)L) | ((uint64_t)R), BW);
    case Instruction::XOR:
      return GetConstant(((uint64_t)L) ^ ((uint64_t)R), BW);
    case Instruction::LSL:
      return GetConstant(((uint64_t)L) << ((uint64_t)R), BW);
    case Instruction::LSR:
      return GetConstant(((uint64_t)L) >> ((uint64_t)R), BW);
    default:
      assert(!"Unreachable");
    }
  }

  Value *EvaluateFloatingPointBinaryConstExpression(
      const double L, const double R, const uint8_t BW,
      const Instruction::IKind Operation) {
    switch (Operation) {
    case Instruction::ADDF:
      return GetConstant(L + R, BW);
    case Instruction::SUBF:
      return GetConstant(L - R, BW);
    case Instruction::MULF:
      return GetConstant(L * R, BW);
    case Instruction::DIVF:
      assert(R != 0.0);
      return GetConstant(L / R, BW);
    default:
      assert(!"Unreachable");
    }
  }

  Value *EvaluateBinaryConstExpression(const Constant *LHS, const Constant *RHS,
                                       const Instruction::IKind Operation) {
    assert(LHS->GetBitWidth() == RHS->GetBitWidth() &&
           "Must have the same bit size");

    switch (Operation) {
    case Instruction::ADD:
    case Instruction::SUB:
    case Instruction::MUL:
    case Instruction::DIV:
    case Instruction::DIVU:
    case Instruction::MOD:
    case Instruction::MODU:
    case Instruction::AND:
    case Instruction::OR:
    case Instruction::XOR:
    case Instruction::LSL:
    case Instruction::LSR:
      return EvaluateIntegerBinaryConstExpression(
          LHS->GetIntValue(), RHS->GetIntValue(), LHS->GetBitWidth(),
          Operation);

      // Floating point
    case Instruction::ADDF:
    case Instruction::SUBF:
    case Instruction::MULF:
    case Instruction::DIVF:
      return EvaluateFloatingPointBinaryConstExpression(
          LHS->GetFloatValue(), RHS->GetFloatValue(), LHS->GetBitWidth(),
          Operation);

    default:
      assert(!"Unreachable");
    }
  }

  Value *CreateBinaryInstruction(Instruction::IKind K, Value *L, Value *R) {
    // If both operand is constant, then evaluate them
    if (L->IsConstant() && R->IsConstant()) {
      auto ConstLHS = dynamic_cast<Constant *>(L);
      auto ConstRHS = dynamic_cast<Constant *>(R);
      assert(ConstLHS && ConstRHS);

      return EvaluateBinaryConstExpression(ConstLHS, ConstRHS, K);
    }

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
  IRFactory(Module &M, TargetMachine *T) : TM(T), CurrentModule(M), ID(0) {}

  Value *CreateAND(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::AND, LHS, RHS);
  }

  Value *CreateOR(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::OR, LHS, RHS);
  }

  Value *CreateXOR(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::XOR, LHS, RHS);
  }

  Value *CreateLSL(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::LSL, LHS, RHS);
  }

  Value *CreateLSR(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::LSR, LHS, RHS);
  }

  Value *CreateADD(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::ADD, LHS, RHS);
  }

  Value *CreateSUB(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::SUB, LHS, RHS);
  }

  Value *CreateMUL(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::MUL, LHS, RHS);
  }

  Value *CreateDIV(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::DIV, LHS, RHS);
  }

  Value *CreateDIVU(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::DIVU, LHS, RHS);
  }

  Value *CreateMOD(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::MOD, LHS, RHS);
  }

  Value *CreateMODU(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::MODU, LHS, RHS);
  }

  Value *CreateADDF(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::ADDF, LHS, RHS);
  }

  Value *CreateSUBF(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::SUBF, LHS, RHS);
  }

  Value *CreateMULF(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::MULF, LHS, RHS);
  }

  Value *CreateDIVF(Value *LHS, Value *RHS) {
    return CreateBinaryInstruction(Instruction::DIVF, LHS, RHS);
  }

  UnaryInstruction *CreateSEXT(Value *Operand, uint8_t BitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(Instruction::SEXT,
                                                   IRType::CreateInt(BitWidth),
                                                   Operand, GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateZEXT(Value *Operand, uint8_t BitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(Instruction::ZEXT,
                                                   IRType::CreateInt(BitWidth),
                                                   Operand, GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateTRUNC(Value *Operand, uint8_t BitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(Instruction::TRUNC,
                                                   IRType::CreateInt(BitWidth),
                                                   Operand, GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateFTOI(Value *Operand, uint8_t BitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(Instruction::FTOI,
                                                   IRType::CreateInt(BitWidth),
                                                   Operand, GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateITOF(Value *Operand, uint8_t BitWidth = 32) {
    auto Inst = std::make_unique<UnaryInstruction>(
        Instruction::ITOF, IRType::CreateFloat(BitWidth), Operand,
        GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  UnaryInstruction *CreateBITCAST(Value *Operand, const IRType &To) {
    auto Inst = std::make_unique<UnaryInstruction>(Instruction::BITCAST, To,
                                                   Operand, GetCurrentBB());
    Inst->SetID(ID++);
    auto InstPtr = Inst.get();
    Insert(std::move(Inst));

    return InstPtr;
  }

  CallInstruction *CreateCALL(std::string &Name, std::vector<Value *> Args,
                              const IRType &Type, int StructIdx = -1) {
    auto Inst = std::make_unique<CallInstruction>(Name, Args, Type,
                                                  GetCurrentBB(), StructIdx);
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

  StackAllocationInstruction *CreateSA(std::string Identifier,
                                       const IRType &Type) {
    auto Inst = std::make_unique<StackAllocationInstruction>(
        Identifier, Type, CurrentModule.GetBB(0));
    auto InstPtr = Inst.get();
    Inst->SetID(ID++);
    CurrentModule.GetBB(0)->InsertSA(std::move(Inst));

    return InstPtr;
  }

  GetElementPointerInstruction *CreateGEP(const IRType &ResultType,
                                          Value *Source, Value *Index) {
    auto Inst = std::make_unique<GetElementPointerInstruction>(
        ResultType, Source, Index, GetCurrentBB());
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

  LoadInstruction *CreateLD(const IRType &ResultType, Value *Source,
                            Value *Offset = nullptr) {
    auto Inst = std::make_unique<LoadInstruction>(ResultType, Source, Offset,
                                                  GetCurrentBB());
    auto InstPtr = Inst.get();
    Inst->SetID(ID++);
    Insert(std::move(Inst));

    return InstPtr;
  }

  MemoryCopyInstruction *CreateMEMCOPY(Value *Destination, Value *Source,
                                       size_t Bytes) {
    auto Inst = std::make_unique<MemoryCopyInstruction>(Destination, Source,
                                                        Bytes, GetCurrentBB());
    auto InstPtr = Inst.get();
    Inst->SetID(ID++);
    Insert(std::move(Inst));

    return InstPtr;
  }

  Value *CreateCMP(CompareInstruction::CompRel Relation, Value *LHS,
                   Value *RHS) {
    // If both operand is constant, then evaluate them
    if (LHS->IsConstant() && RHS->IsConstant()) {
      assert(LHS->IsFPType() == RHS->IsFPType());
      assert(LHS->GetBitWidth() == RHS->GetBitWidth());
      auto ConstLHS = dynamic_cast<Constant *>(LHS);
      auto ConstRHS = dynamic_cast<Constant *>(RHS);
      auto TrueVal = GetConstant((uint64_t)1);
      auto FalseVal = GetConstant((uint64_t)0);

      // Integer comparison
      if (!LHS->IsFPType() && !RHS->IsFPType()) {
        const auto L = ConstLHS->GetIntValue();
        const auto R = ConstRHS->GetIntValue();

        return EvaluateComparison(L, R, Relation, TrueVal, FalseVal);
      } else { // Float comparison
        const auto L = ConstLHS->GetFloatValue();
        const auto R = ConstRHS->GetFloatValue();

        return EvaluateComparison(L, R, Relation, TrueVal, FalseVal);
      }
    }

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

  GlobalVariable *CreateGlobalVar(std::string &Identifier, const IRType &Type) {
    auto GlobalVar = new GlobalVariable(Identifier, Type);
    GlobalVar->SetID(ID++);

    return GlobalVar;
  }

  GlobalVariable *CreateGlobalVar(std::string &Identifier, const IRType &Type,
                                  std::string Value) {
    auto GlobalVar = new GlobalVariable(Identifier, Type, std::move(Value));
    GlobalVar->SetID(ID++);

    return GlobalVar;
  }

  GlobalVariable *CreateGlobalVar(std::string &Identifier, const IRType &Type,
                                  Value *Val) {
    auto GlobalVar = new GlobalVariable(Identifier, Type, Val);
    GlobalVar->SetID(ID++);

    return GlobalVar;
  }

  GlobalVariable *CreateGlobalVar(std::string &Identifier, const IRType &Type,
                                  std::vector<uint64_t> InitList) {
    auto GlobalVar = new GlobalVariable(Identifier, Type, std::move(InitList));
    GlobalVar->SetID(ID++);

    return GlobalVar;
  }

  void CreateNewFunction(std::string &Name, IRType ReturnType) {
    CurrentModule.AddFunction(Function(Name, std::move(ReturnType)));
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

  void EraseLastBB() { GetCurrentFunction()->GetBasicBlocks().pop_back(); }

  void EraseInst(Instruction *I) {
    for (auto &BB : GetCurrentFunction()->GetBasicBlocks())
      for (size_t i = 0; i < BB->GetInstructions().size(); i++)
        if (BB->GetInstructions()[i].get() == I) {
          BB->GetInstructions().erase(BB->GetInstructions().begin() + i);
          return;
        }
  }

  void EraseLastInst() {
    GetCurrentFunction()->GetBasicBlocks().back()->GetInstructions().pop_back();
  }

  void AddToSymbolTable(std::string &Identifier, Value *Value) {
    SymbolTable[Identifier] = Value;
  }

  Value *GetSymbolValue(const std::string &Identifier) {
    return SymbolTable[Identifier];
  }

  Constant *GetConstant(uint64_t C, uint8_t BW = 32) {
    std::pair<uint64_t, uint8_t> RequestedConst = {C, BW};

    if (auto ConstVal = IntConstantPool[RequestedConst].get();
        ConstVal != nullptr)
      return ConstVal;

    IntConstantPool[RequestedConst] =
        std::make_unique<Constant>(RequestedConst.first, RequestedConst.second);
    return IntConstantPool[RequestedConst].get();
  }

  Constant *GetConstant(double C, uint8_t BitWidth = 64) {
    if (auto ConstVal = FPConstantPool[C].get(); ConstVal != nullptr)
      return ConstVal;

    FPConstantPool[C] = std::make_unique<Constant>(C, BitWidth);
    return FPConstantPool[C].get();
  }

  std::vector<BasicBlock *> &GetLoopIncrementBBsTable() {
    return LoopIncrementBBsTable;
  }

  std::vector<BasicBlock *> &GetBreaksEndBBsTable() {
    return BreaksTargetBBsTable;
  }

  TargetMachine *GetTargetMachine() { return TM; }

private:
  TargetMachine *TM{};

  Module &CurrentModule;

  /// A counter essentially, which used to give Values a unique ID.
  unsigned ID;

  /// Shows whether we are in the global scope or not.
  bool GlobalScope = false;

  /// To store already created integer constants
  std::map<std::pair<uint64_t, uint8_t>, std::unique_ptr<Constant>>
      IntConstantPool;

  /// To store already created floating point constants
  std::map<double, std::unique_ptr<Constant>> FPConstantPool;

  // TODO: Consider putting these to Function class

  /// Hold the local symbols for the current function.
  std::map<std::string, Value *> SymbolTable;

  /// To keep track how many times each label were defined. This number
  /// can be used to concatenate it to the label to make it unique.
  std::map<std::string, unsigned> LabelTable;

  /// For context information for "continue" statements. Containing the pointer
  /// to the basic block which will be the target of the generated jump.
  std::vector<BasicBlock *> LoopIncrementBBsTable;

  /// For context information for "break" statements. Containing the pointer
  /// to the basic block which will be the target of the generated jump.
  std::vector<BasicBlock *> BreaksTargetBBsTable;
};

#endif
