#include "IRtoLLIR.hpp"
#include "../middle_end/IR/BasicBlock.hpp"
#include "../middle_end/IR/Function.hpp"
#include "LowLevelType.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include <cassert>
#include "Support.hpp"

MachineOperand GetMachineOperandFromValue(Value *Val, TargetMachine *TM) {
  if (Val->IsRegister()) {
    auto BitWidth = Val->GetBitWidth();
    auto VReg = MachineOperand::CreateVirtualRegister(Val->GetID());

    if (Val->GetTypeRef().IsPTR())
      VReg.SetType(LowLevelType::CreatePTR(TM->GetPointerSize()));
    else
      VReg.SetType(LowLevelType::CreateINT(BitWidth));

    return VReg;
  } else if (Val->IsParameter()) {
    auto Result = MachineOperand::CreateParameter(Val->GetID());
    auto BitWidth = Val->GetBitWidth();
    // FIXME: Only handling int params now, handle others too
    // And add type to registers and others too
    if (Val->GetTypeRef().IsPTR())
      Result.SetType(LowLevelType::CreatePTR(TM->GetPointerSize()));
    else
      Result.SetType(LowLevelType::CreateINT(BitWidth));

    return Result;
  } else if (Val->IsConstant()) {
    auto C = dynamic_cast<Constant *>(Val);
    assert(!C->IsFPConst() && "TODO");
    auto Result = MachineOperand::CreateImmediate(C->GetIntValue());
    Result.SetType(LowLevelType::CreateINT(32));
    return Result;
  } else {
    assert(!"Unhandled MO case");
  }

  return MachineOperand();
}

MachineInstruction IRtoLLIR::ConvertToMachineInstr(Instruction *Instr,
                                        MachineBasicBlock *BB,
                                        std::vector<MachineBasicBlock> &BBs) {
  auto Operation = Instr->GetInstructionKind();
  auto ParentFunction = BB->GetParent();

  auto ResultMI = MachineInstruction((unsigned)Operation + (1 << 16), BB);

  // Three address ALU instructions: INSTR Result, Op1, Op2
  if (auto I = dynamic_cast<BinaryInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue((Value *)I, TM);
    auto FirstSrcOp = GetMachineOperandFromValue(I->GetLHS(), TM);
    auto SecondSrcOp = GetMachineOperandFromValue(I->GetRHS(), TM);

    ResultMI.AddOperand(Result);
    ResultMI.AddOperand(FirstSrcOp);
    ResultMI.AddOperand(SecondSrcOp);
  }
  // Two address ALU instructions: INSTR Result, Op
  else if (auto I = dynamic_cast<UnaryInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue((Value *)I, TM);
    auto Op = GetMachineOperandFromValue(I->GetOperand(), TM);

    ResultMI.AddOperand(Result);
    ResultMI.AddOperand(Op);
  }
  // Store instruction: STR [address], Src
  else if (auto I = dynamic_cast<StoreInstruction *>(Instr); I != nullptr) {
    // FIXME: maybe it should be something else then a register since its
    // an address, revisit this
    assert((I->GetMemoryLocation()->IsRegister() ||
           I->GetMemoryLocation()->IsGlobalVar()) && "Forbidden destination");

    unsigned GlobAddrReg;
    unsigned AddressReg;
    if (I->GetMemoryLocation()->IsGlobalVar()) {
      auto GlobalAddress = MachineInstruction(MachineInstruction::GLOBAL_ADDRESS, BB);
      GlobAddrReg = ParentFunction->GetNextAvailableVReg();
      GlobalAddress.AddVirtualRegister(GlobAddrReg, TM->GetPointerSize());
      GlobalAddress.AddGlobalSymbol(((GlobalVariable*)I->GetMemoryLocation())->GetName());
      BB->InsertInstr(GlobalAddress);
      AddressReg = GlobAddrReg;
    } else
      AddressReg = I->GetMemoryLocation()->GetID();

    ResultMI.AddAttribute(MachineInstruction::IS_STORE);

    // Check if the instruction accessing the stack
    if (ParentFunction->IsStackSlot(AddressReg))
      // if it is then set the operand to a stack access
      ResultMI.AddStackAccess(AddressReg);
    else // otherwise a normal memory access
      ResultMI.AddMemory(AddressReg);

    // if the source is a struct and not a struct pointer
    if (I->GetSavedValue()->GetTypeRef().IsStruct() &&
        !I->GetSavedValue()->GetTypeRef().IsPTR()) {
      // Handle the case where the referred struct is a function parameter,
      // therefore held in registers
      if (auto FP = dynamic_cast<FunctionParameter *>(I->GetSavedValue()); FP != nullptr) {
        unsigned RegSize = TM->GetPointerSize();
        auto StructName = FP->GetName();
        assert(!StructToRegMap[StructName].empty() && "Unknown struct name");

        MachineInstruction CurrentStore;
        unsigned Counter = 0;
        // Create stores for the register which holds the struct parts
        for (auto ParamID : StructToRegMap[StructName]) {
          CurrentStore = MachineInstruction(MachineInstruction::STORE, BB);
          CurrentStore.AddStackAccess(AddressReg, Counter * RegSize / 8);
          CurrentStore.AddVirtualRegister(ParamID, RegSize);
          Counter++;
          // insert all the stores but the last one, that will be the return value
          if (Counter < StructToRegMap[StructName].size())
            BB->InsertInstr(CurrentStore);
        }
        return CurrentStore;
      }
      // Handle other cases, like when the structure is a return value from a
      // function
      else {
        // determine how much register is used to hold the return val
        unsigned StructBitSize = (I->GetSavedValue()->GetTypeRef().GetByteSize() * 8);
        unsigned MaxRegSize = TM->GetPointerSize();
        unsigned RegsCount = GetNextAlignedValue(StructBitSize, MaxRegSize) / MaxRegSize;
        auto &RetRegs = TM->GetABI()->GetReturnRegisters();
        assert(RegsCount <= RetRegs.size());

        MachineInstruction Store;
        for (size_t i = 0; i < RegsCount; i++) {
          Store = MachineInstruction(MachineInstruction::STORE, BB);
          Store.AddStackAccess(AddressReg, (TM->GetPointerSize() / 8) * i);
          Store.AddRegister(RetRegs[i]->GetID(), TM->GetPointerSize());
          if (i == (RegsCount - 1))
            return Store;
          BB->InsertInstr(Store);
        }
      }
    } else
      ResultMI.AddOperand(GetMachineOperandFromValue(I->GetSavedValue(), TM));
  }
  // Load instruction: LD Dest, [address]
  else if (auto I = dynamic_cast<LoadInstruction *>(Instr); I != nullptr) {
    // FIXME: same as with STORE
    assert((I->GetMemoryLocation()->IsRegister() ||
            I->GetMemoryLocation()->IsGlobalVar()) && "Forbidden source");

    unsigned GlobAddrReg;
    unsigned AddressReg;
    if (I->GetMemoryLocation()->IsGlobalVar()) {
      auto GlobalAddress = MachineInstruction(MachineInstruction::GLOBAL_ADDRESS, BB);
      GlobAddrReg = ParentFunction->GetNextAvailableVReg();
      GlobalAddress.AddVirtualRegister(GlobAddrReg, TM->GetPointerSize());
      GlobalAddress.AddGlobalSymbol(((GlobalVariable*)I->GetMemoryLocation())->GetName());
      BB->InsertInstr(GlobalAddress);
      AddressReg = GlobAddrReg;
    } else
      AddressReg = I->GetMemoryLocation()->GetID();

    ResultMI.AddAttribute(MachineInstruction::IS_LOAD);
    ResultMI.AddOperand(GetMachineOperandFromValue((Value *)I, TM));

    // Check if the instruction accessing the stack
    if (ParentFunction->IsStackSlot(AddressReg))
      // if it is then set the operand to a stack access
      ResultMI.AddStackAccess(AddressReg);
    else // otherwise a normal memory access
      ResultMI.AddMemory(AddressReg);
  }
  // GEP instruction: GEP Dest, Source, list of indexes
  // to
  //   STACK_ADDRESS Dest, Source # Or GLOBAL_ADDRESS if Source is global
  // **arithmetic instructions to calculate the index** ex: 1 index which is 6
  //   MUL idx, sizeof(Source[0]), 6
  //   ADD Dest, Dest, idx
  else if (auto I = dynamic_cast<GetElementPointerInstruction *>(Instr); I != nullptr) {
    MachineInstruction GoalInstr;

    const bool IsGlobal = I->GetSource()->IsGlobalVar();
    if (IsGlobal)
      GoalInstr = MachineInstruction(MachineInstruction::GLOBAL_ADDRESS, BB);
    else
      GoalInstr = MachineInstruction(MachineInstruction::STACK_ADDRESS, BB);

    auto Dest = GetMachineOperandFromValue((Value *)I, TM);
    GoalInstr.AddOperand(Dest);

    //assert(ParentFunction->IsStackSlot(AddressReg) && "Must be stack slot");
    if (IsGlobal)
      GoalInstr.AddGlobalSymbol(((GlobalVariable*)I->GetSource())->GetName());
    else
      GoalInstr.AddStackAccess(I->GetSource()->GetID());

    auto &SourceType = I->GetSource()->GetTypeRef();
    unsigned ConstantIndexPart = 0;
    auto Index = ((Constant*)I->GetIndex())->GetIntValue();
    if (I->GetIndex()->IsConstant()) {
      if (!SourceType.IsStruct())
        ConstantIndexPart = (SourceType.CalcElemSize(0) * Index);
      else // its a struct and has to determine the offset other way
          ConstantIndexPart = SourceType.GetElemByteOffset(Index);
    } else
        assert(!"Unimplemented for expression indexes");

    // If there is nothing to add, then exit now
    if (ConstantIndexPart == 0)
      return GoalInstr;

    BB->InsertInstr(GoalInstr);

    auto ADD = MachineInstruction(MachineInstruction::ADD, BB);
    ADD.AddOperand(Dest);
    ADD.AddOperand(Dest);
    ADD.AddImmediate(ConstantIndexPart, Dest.GetSize());

    return ADD;
  }
  // Jump instruction: J label
  else if (auto I = dynamic_cast<JumpInstruction *>(Instr); I != nullptr) {
    for (auto &BB : BBs)
      if (I->GetTargetLabelName() == BB.GetName()) {
        ResultMI.AddLabel(BB.GetName().c_str());
        break;
      }
  }
  // Branch instruction: Br op label label
  else if (auto I = dynamic_cast<BranchInstruction *>(Instr); I != nullptr) {
    const char *LabelTrue = nullptr;
    const char *LabelFalse = nullptr;

    for (auto &BB : BBs) {
      if (LabelTrue == nullptr && I->GetTrueLabelName() == BB.GetName())
        LabelTrue = BB.GetName().c_str();

      if (LabelFalse == nullptr && I->HasFalseLabel() &&
          I->GetFalseLabelName() == BB.GetName())
        LabelFalse = BB.GetName().c_str();
    }

    ResultMI.AddOperand(GetMachineOperandFromValue(I->GetCondition(), TM));
    ResultMI.AddLabel(LabelTrue);
    if (I->HasFalseLabel())
      ResultMI.AddLabel(LabelTrue);
  }
  // Compare instruction: ret op
  else if (auto I = dynamic_cast<CompareInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue((Value *)I, TM);
    auto FirstSrcOp = GetMachineOperandFromValue(I->GetLHS(), TM);
    auto SecondSrcOp = GetMachineOperandFromValue(I->GetRHS(), TM);

    ResultMI.AddOperand(Result);
    ResultMI.AddOperand(FirstSrcOp);
    ResultMI.AddOperand(SecondSrcOp);

    ResultMI.SetAttributes(I->GetRelation());
  }
  // Call instruction: call Result, function_name(Param1, ...)
  else if (auto I = dynamic_cast<CallInstruction *>(Instr); I != nullptr) {
    // The function has a call instruction
    ParentFunction->SetToCaller();

    // insert COPY/MOV -s for each Param to move them the right registers
    // ignoring the case when there is too much parameter and has to pass
    // some parameters on the stack
    auto &TargetArgRegs = TM->GetABI()->GetArgumentRegisters();
    unsigned ParamCounter = 0;
    for (auto *Param : I->GetArgs()) {
      MachineInstruction Instr;

      if (Param->GetTypeRef().IsStruct()) {
        // how many register are used to pass this struct
        unsigned StructBitSize = (Param->GetTypeRef().GetByteSize() * 8);
        unsigned MaxRegSize = TM->GetPointerSize();
        unsigned RegsCount = GetNextAlignedValue(StructBitSize, MaxRegSize) / MaxRegSize;

        for (size_t i = 0; i < RegsCount; i++) {
          Instr = MachineInstruction(MachineInstruction::LOAD, BB);
          Instr.AddRegister(TargetArgRegs[ParamCounter]->GetID(),
                            TargetArgRegs[ParamCounter]->GetBitWidth());
          Instr.AddStackAccess(Param->GetID(), i * (TM->GetPointerSize() / 8));
          BB->InsertInstr(Instr);
          ParamCounter++;
        }
      } else {
        Instr = MachineInstruction(MachineInstruction::MOV, BB);

        Instr.AddRegister(TargetArgRegs[ParamCounter]->GetID(),
                          TargetArgRegs[ParamCounter]->GetBitWidth());

        Instr.AddOperand(GetMachineOperandFromValue(Param, TM));
        BB->InsertInstr(Instr);
        ParamCounter++;
      }
    }

    ResultMI.AddFunctionName(I->GetName().c_str());
  }
  // Ret instruction: ret op
  else if (auto I = dynamic_cast<ReturnInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue(I->GetRetVal(), TM);
    ResultMI.AddOperand(Result);

    // insert load to load in the return val to the return registers
    auto &TargetRetRegs = TM->GetABI()->GetReturnRegisters();
    if (I->GetRetVal()->GetTypeRef().IsStruct()) {
      // how many register are used to pass this struct
      unsigned StructBitSize = (I->GetRetVal()->GetTypeRef().GetByteSize() * 8);
      unsigned MaxRegSize = TM->GetPointerSize();
      unsigned RegsCount = GetNextAlignedValue(StructBitSize, MaxRegSize) / MaxRegSize;

      for (size_t i = 0; i < RegsCount; i++) {
        auto Instr = MachineInstruction(MachineInstruction::LOAD, BB);
        Instr.AddRegister(TargetRetRegs[i]->GetID(),
                          TargetRetRegs[i]->GetBitWidth());
        Instr.AddStackAccess(I->GetRetVal()->GetID(), i * (TM->GetPointerSize() / 8));
        BB->InsertInstr(Instr);
      }
    }
    else if (I->GetRetVal()->IsConstant()) {
      auto &RetRegs = TM->GetABI()->GetReturnRegisters();

      auto LoadImm = MachineInstruction(MachineInstruction::LOAD_IMM, BB);
      LoadImm.AddRegister(RetRegs[0]->GetID(), RetRegs[0]->GetBitWidth());
      LoadImm.AddOperand(GetMachineOperandFromValue(I->GetRetVal(), TM));
      BB->InsertInstr(LoadImm);
    }
  }
  // Memcopy instruction: memcopy dest, source, bytes_number
  else if (auto I = dynamic_cast<MemoryCopyInstruction *>(Instr); I != nullptr) {
    // lower this into load and store pairs if used with structs lower then
    // a certain size (for now be it the size which can be passed by value)
    // otherwise create a call maybe to an intrinsic memcopy function
    for (size_t i = 0; i < (I->GetSize() / /* TODO: use alignment here */ 4); i++) {
      auto Load = MachineInstruction(MachineInstruction::LOAD, BB);
      auto NewVReg = ParentFunction->GetNextAvailableVReg();
      Load.AddRegister(NewVReg, /* TODO: use alignment here */ 32);
      Load.AddStackAccess(I->GetSource()->GetID(), i * /* TODO: use alignment here */ 4);
      BB->InsertInstr(Load);

      auto Store = MachineInstruction(MachineInstruction::STORE, BB);
      Store.AddStackAccess(I->GetDestination()->GetID(), i * /* TODO: use alignment here */ 4);
      Store.AddRegister(NewVReg, /* TODO: use alignment here */ 32);
      // TODO: Change the function so it does not return the instruction but
      // insert it in the function so don't have to do these annoying returns
      if (i == ((I->GetSize() / /* TODO: use alignment here */ 4) - 1))
        return Store;
      BB->InsertInstr(Store);
    }
  } else
    assert(!"Unimplemented instruction!");

  return ResultMI;
}

/// For each stack allocation instruction insert a new entry into the StackFrame
void HandleStackAllocation(StackAllocationInstruction *Instr,
                           MachineFunction *Func) {
  Func->InsertStackSlot(Instr->GetID(), Instr->GetType().GetByteSize());
}

void IRtoLLIR::HandleFunctionParams(Function &F, MachineFunction *Func) {
  for (auto &Param : F.GetParameters()) {
    auto ParamID = Param->GetID();
    auto ParamSize = Param->GetBitWidth();

    // Handle structs
    if (Param->GetTypeRef().IsStruct()) {
      auto StructName = Param->GetName();
      // Pointer size also represents the architecture bit size and more
      // importantly the largest bitwidth a general register can have for the
      // given target
      // TODO: revisit this statement later and refine the implementation
      // for example have a function which check all registers and decide the
      // max size that way, or the max possible size of parameter registers
      // but for AArch64 and RISC-V its sure the bit size of the architecture

      // FIXME: The maximum allowed structure size which allowed to be passed
      // by the target is target dependent. Remove the hardcoded value and
      // ask the target for the right size.
      unsigned MaxStructSize = 128; // bit size
      for (size_t i = 0; i < MaxStructSize / TM->GetPointerSize(); i++) {
        // FIXME: for now adding 1000000 is kind of arbitrary, the point is to
        // create a surely unique ID. Should be done in a better way its ok
        // for now.
        StructToRegMap[StructName].push_back(ParamID + 1000000 + i);
        Func->InsertParameter(ParamID + 1000000 + i,
                              LowLevelType::CreateINT(TM->GetPointerSize()));
      }

      continue;
    }

    if (Param->GetTypeRef().IsPTR())
      Func->InsertParameter(ParamID, LowLevelType::CreatePTR(TM->GetPointerSize()));
    else
      Func->InsertParameter(ParamID, LowLevelType::CreateINT(ParamSize));
  }
}

void IRtoLLIR::GenerateLLIRFromIR() {
  for (auto &Fun : IRM.GetFunctions()) {
    auto NewMachineFunc = MachineFunction{};
    std::vector<MachineBasicBlock> MBBs;

    TU->AddFunction(NewMachineFunc);
    MachineFunction *MFunction = TU->GetCurrentFunction();

    MFunction->SetName(Fun.GetName());
    HandleFunctionParams(Fun, MFunction);

    // Create all basic block first with their name, so jumps can refer to them
    // already
    for (auto &BB : Fun.GetBasicBlocks()) {
      MBBs.push_back(MachineBasicBlock{BB.get()->GetName(), MFunction});
    }

    // Assign the basic block to the new function
    MFunction->SetBasicBlocks(std::move(MBBs));
    auto &MFuncMBBs = MFunction->GetBasicBlocks();

    unsigned BBCounter = 0;
    for (auto &BB : Fun.GetBasicBlocks()) {
      for (auto &Instr : BB->GetInstructions()) {
        auto InstrPtr = Instr.get();

        if (InstrPtr->IsStackAllocation()) {
          HandleStackAllocation((StackAllocationInstruction *)InstrPtr,
                                MFunction);
          continue;
        }
        MFuncMBBs[BBCounter].InsertInstr(
            ConvertToMachineInstr(InstrPtr, &MFuncMBBs[BBCounter], MFuncMBBs));
      }

      BBCounter++;
    }
  }
  for (auto &GlobalVar : IRM.GetGlobalVars()) {
    auto Name = ((GlobalVariable*)GlobalVar.get())->GetName();
    auto Size = GlobalVar->GetTypeRef().GetByteSize();

    auto GD = GlobalData(Name, Size);
    auto &InitList = ((GlobalVariable*)GlobalVar.get())->GetInitList();

    if (GlobalVar->GetTypeRef().IsStruct() || GlobalVar->GetTypeRef().IsArray()) {
      // If the init list is empty, then just allocate Size amount of zeros
      if (InitList.empty())
        GD.InsertAllocation(Size, 0);
      // if the list is not empty then allocate the appropriate type of memories
      // with initialization
      else {
        // struct case
        if (GlobalVar->GetTypeRef().IsStruct()) {
          size_t InitListIndex = 0;
          for (auto &MemberType : GlobalVar->GetTypeRef().GetMemberTypes()) {
            assert(InitListIndex < InitList.size());
            GD.InsertAllocation(MemberType.GetByteSize(),
                                InitList[InitListIndex]);
            InitListIndex++;
          }
        }
        // array case
        else {
          const auto Size = GlobalVar->GetTypeRef().GetBaseType().GetByteSize();
          for (auto InitVal : InitList)
            GD.InsertAllocation(Size, InitVal);
        }
      }
    }
    // scalar case
    else if (InitList.empty())
      GD.InsertAllocation(Size, 0);
    else {
      GD.InsertAllocation(Size, InitList[0]);
    }

    TU->AddGlobalData(GD);
  }
}
