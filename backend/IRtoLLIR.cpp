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

MachineOperand IRtoLLIR::GetMachineOperandFromValue(Value *Val,
                                                    MachineBasicBlock *MBB,
                                                    bool IsDef = false) {
  assert(Val);
  assert(MBB);
  auto MF = MBB->GetParent();
  assert(MF);

  if (Val->IsRegister()) {
    auto BitWidth = Val->GetBitWidth();
    if (Val->GetTypeRef().IsPTR() && dynamic_cast<StackAllocationInstruction*>(Val) == nullptr)
      BitWidth = TM->GetPointerSize();
    unsigned NextVReg;

    // If the register were spilled, (example: function return values are
    // spilled to the stack) then load the value in first into a VReg
    // and return this VReg as LLIR VReg.
    // TODO: Investigate if this is the appropriate place and way to do this
    if (!IsDef && IRVregToLLIRVreg.count(Val->GetID()) == 0 &&
        MF->IsStackSlot(Val->GetID())) {
      auto Instr = MachineInstruction(MachineInstruction::LOAD,
                                      &MF->GetBasicBlocks().back());
      NextVReg = MF->GetNextAvailableVReg();
      Instr.AddVirtualRegister(NextVReg, BitWidth);
      Instr.AddStackAccess(Val->GetID());
      MBB->InsertInstr(Instr);
    }
    // If the IR VReg is mapped already to an LLIR VReg then use that
    else if (IRVregToLLIRVreg.count(Val->GetID()) > 0) {
      if (!IsDef && MF->IsStackSlot(IRVregToLLIRVreg[Val->GetID()])) {
        auto Instr = MachineInstruction(MachineInstruction::LOAD,
                                        &MF->GetBasicBlocks().back());
        NextVReg = MF->GetNextAvailableVReg();
        Instr.AddVirtualRegister(NextVReg, BitWidth);
        Instr.AddStackAccess(IRVregToLLIRVreg[Val->GetID()]);
        MBB->InsertInstr(Instr);
      } else
        NextVReg = IRVregToLLIRVreg[Val->GetID()];
    }
    // Otherwise get the next available LLIR VReg and create a mapping entry
    else {
      NextVReg = MF->GetNextAvailableVReg();
      IRVregToLLIRVreg[Val->GetID()] = NextVReg;
    }

    auto VReg = MachineOperand::CreateVirtualRegister(NextVReg);

    if (Val->GetTypeRef().IsPTR())
      VReg.SetType(LowLevelType::CreatePTR(TM->GetPointerSize()));
    else
      VReg.SetType(LowLevelType::CreateScalar(BitWidth));

    return VReg;
  } else if (Val->IsParameter()) {
    auto Result = MachineOperand::CreateParameter(Val->GetID());
    auto BitWidth = Val->GetBitWidth();
    // FIXME: Only handling int params now, handle others too
    // And add type to registers and others too
    if (Val->GetTypeRef().IsPTR())
      Result.SetType(LowLevelType::CreatePTR(TM->GetPointerSize()));
    else
      Result.SetType(LowLevelType::CreateScalar(BitWidth));

    return Result;
  } else if (Val->IsConstant()) {
    auto C = dynamic_cast<Constant *>(Val);

    MachineOperand Result =
        C->IsFPConst() ? MachineOperand::CreateFPImmediate(C->GetFloatValue())
                       : MachineOperand::CreateImmediate(C->GetIntValue());

    Result.SetType(LowLevelType::CreateScalar(C->GetBitWidth()));

    return Result;
  } else if (Val->IsGlobalVar()) {
    auto Instr = MachineInstruction(MachineInstruction::GLOBAL_ADDRESS, MBB);
    auto NextVReg = MF->GetNextAvailableVReg();
    Instr.AddVirtualRegister(NextVReg, TM->GetPointerSize());
    Instr.AddGlobalSymbol(((GlobalVariable *)Val)->GetName());
    MBB->InsertInstr(Instr);

    auto MO = MachineOperand::CreateVirtualRegister(NextVReg);
    MO.SetType(LowLevelType::CreatePTR(TM->GetPointerSize()));

    return MO;
  } else {
    assert(!"Unhandled MO case");
  }

  return MachineOperand();
}

unsigned IRtoLLIR::GetIDFromValue(Value * Val) {
  assert(Val);
  unsigned Ret = IRVregToLLIRVreg.count(Val->GetID()) > 0 ?
  IRVregToLLIRVreg[Val->GetID()] : Val->GetID();
  return Ret;
}

MachineInstruction IRtoLLIR::ConvertToMachineInstr(Instruction *Instr,
                                        MachineBasicBlock *BB,
                                        std::vector<MachineBasicBlock> &BBs) {
  auto Operation = Instr->GetInstructionKind();
  auto ParentFunction = BB->GetParent();

  auto ResultMI = MachineInstruction((unsigned)Operation + (1 << 16), BB);

  // Three address ALU instructions: INSTR Result, Op1, Op2
  if (auto I = dynamic_cast<BinaryInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue((Value *)I, BB, true);
    auto FirstSrcOp = GetMachineOperandFromValue(I->GetLHS(), BB);
    auto SecondSrcOp = GetMachineOperandFromValue(I->GetRHS(), BB);

    ResultMI.AddOperand(Result);
    ResultMI.AddOperand(FirstSrcOp);
    ResultMI.AddOperand(SecondSrcOp);
  }
  // Two address ALU instructions: INSTR Result, Op
  else if (auto I = dynamic_cast<UnaryInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue((Value *)I, BB, true);
    MachineOperand Op;

    if (Operation == Instruction::BITCAST) {
      // if a ptr to ptr cast end both pointer at the same ptr level
      // ex: i32* to i8*, then issue a STACK_ADDRESS instruction
      if (I->GetTypeRef().IsPTR() && I->GetOperand()->GetTypeRef().IsPTR() &&
          I->GetTypeRef().GetPointerLevel() ==
              I->GetOperand()->GetTypeRef().GetPointerLevel() &&
          ParentFunction->IsStackSlot(I->GetOperand()->GetID())) {
        if (SpilledReturnValuesStackIDs.count(I->GetOperand()->GetID()) == 0) {
          ResultMI.SetOpcode(MachineInstruction::STACK_ADDRESS);
          Op = MachineOperand::CreateStackAccess(I->GetOperand()->GetID());
        }
        // If the stack slot is actually a spilled return value, then the cast
        // actually if for the spilled value, therefore a load must be issued.
        // Also for casting pointer to pointers at this level no more
        // instruction is required, therefore the single load is enough here.
        else {
          ResultMI.SetOpcode(MachineInstruction::LOAD);
          Op = MachineOperand::CreateStackAccess(GetIDFromValue(I->GetOperand()));
        }
      } else {
        // otherwise use a move
        ResultMI.SetOpcode(MachineInstruction::MOV);
        Op = GetMachineOperandFromValue(
            I->GetOperand(), BB,
            (unsigned)Operation == (unsigned)MachineInstruction::BITCAST);
      }
    }
     else
      Op = GetMachineOperandFromValue(I->GetOperand(), BB);

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
    } else {
      AddressReg = GetIDFromValue(I->GetMemoryLocation());
    }

    ResultMI.AddAttribute(MachineInstruction::IS_STORE);

    // Check if the instruction accessing the stack
    if (ParentFunction->IsStackSlot(AddressReg))
      // if it is then set the operand to a stack access
      ResultMI.AddStackAccess(AddressReg);
    else // otherwise a normal memory access
      ResultMI.AddMemory(AddressReg, TM->GetPointerSize());

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
        const unsigned StructBitSize =
            (I->GetSavedValue()->GetTypeRef().GetBaseTypeByteSize() * 8);
        const unsigned MaxRegSize = TM->GetPointerSize();
        const unsigned RegsCount =
            GetNextAlignedValue(StructBitSize, MaxRegSize) / MaxRegSize;
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
    } else if (I->GetSavedValue()->IsGlobalVar()) {
        auto GlobalAddress = MachineInstruction(MachineInstruction::GLOBAL_ADDRESS, BB);
        auto SourceReg = ParentFunction->GetNextAvailableVReg();
        GlobalAddress.AddVirtualRegister(SourceReg, TM->GetPointerSize());
        GlobalAddress.AddGlobalSymbol(((GlobalVariable*)I->GetSavedValue())->GetName());
        BB->InsertInstr(GlobalAddress);
        ResultMI.AddVirtualRegister(SourceReg, TM->GetPointerSize());
    }
    // if the source is a SA instruction, then its address which needs to be
    // stored, therefore it has to be materialized by STACK_ADDRESS instruction
    else if (dynamic_cast<StackAllocationInstruction *>(I->GetSavedValue())) {
      assert(ParentFunction->IsStackSlot(I->GetSavedValue()->GetID()));
      auto SA = MachineInstruction(MachineInstruction::STACK_ADDRESS, BB);
      auto SourceReg = ParentFunction->GetNextAvailableVReg();
      SA.AddVirtualRegister(SourceReg, TM->GetPointerSize());
      SA.AddStackAccess(I->GetSavedValue()->GetID());
      BB->InsertInstr(SA);
      ResultMI.AddVirtualRegister(SourceReg, TM->GetPointerSize());
    } else
      ResultMI.AddOperand(GetMachineOperandFromValue(I->GetSavedValue(), BB));
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
    } else {
      AddressReg = GetIDFromValue(I->GetMemoryLocation());
    }

    ResultMI.AddAttribute(MachineInstruction::IS_LOAD);
    ResultMI.AddOperand(GetMachineOperandFromValue((Value *)I, BB, true));

    // Check if the instruction accessing the stack
    if (ParentFunction->IsStackSlot(AddressReg))
      // if it is then set the operand to a stack access
      ResultMI.AddStackAccess(AddressReg);
    else // otherwise a normal memory access
      ResultMI.AddMemory(AddressReg, TM->GetPointerSize());

    // if the destination is a struct and not a struct pointer
    if (I->GetTypeRef().IsStruct() && !I->GetTypeRef().IsPTR()) {
      unsigned StructBitSize = (I->GetTypeRef().GetByteSize() * 8);
      unsigned RegSize = TM->GetPointerSize();
      unsigned RegsCount = GetNextAlignedValue(StructBitSize, RegSize) / RegSize;

      // Create loads for the registers which holds the struct parts
      for (size_t i = 0; i < RegsCount; i++) {
        auto CurrentLoad = MachineInstruction(MachineInstruction::LOAD, BB);
        auto NewVReg = ParentFunction->GetNextAvailableVReg();

        CurrentLoad.AddVirtualRegister(NewVReg, RegSize);
        StructByIDToRegMap[I->GetID()].push_back(NewVReg);
        CurrentLoad.AddStackAccess(AddressReg, i * RegSize / 8);

        // insert all the stores but the last one, that will be the return value
        if (i + 1 < RegsCount)
          BB->InsertInstr(CurrentLoad);
        else
          return CurrentLoad;
      }
    }
  }
  // GEP instruction: GEP Dest, Source, list of indexes
  // to
  //   STACK_ADDRESS Dest, Source # Or GLOBAL_ADDRESS if Source is global
  // **arithmetic instructions to calculate the index** ex: 1 index which is 6
  //   MUL idx, sizeof(Source[0]), 6
  //   ADD Dest, Dest, idx
  else if (auto I = dynamic_cast<GetElementPointerInstruction *>(Instr); I != nullptr) {
    MachineInstruction GoalInstr;

    auto SourceID = GetIDFromValue(I->GetSource());
    const bool IsGlobal = I->GetSource()->IsGlobalVar();
    const bool IsStack = ParentFunction->IsStackSlot(SourceID);
    const bool IsReg = !IsGlobal && !IsStack;

    if (IsGlobal)
      GoalInstr = MachineInstruction(MachineInstruction::GLOBAL_ADDRESS, BB);
    else if (IsStack)
      GoalInstr = MachineInstruction(MachineInstruction::STACK_ADDRESS, BB);

    auto Dest = GetMachineOperandFromValue((Value *)I, BB, true);
    GoalInstr.AddOperand(Dest);

    if (IsGlobal)
      GoalInstr.AddGlobalSymbol(((GlobalVariable*)I->GetSource())->GetName());
    else if (IsStack)
      GoalInstr.AddStackAccess(SourceID);

    auto &SourceType = I->GetSource()->GetTypeRef();
    unsigned ConstantIndexPart = 0;
    bool IndexIsInReg = false;
    unsigned MULResVReg = 0;
    auto IndexReg = GetMachineOperandFromValue(I->GetIndex(), BB);
    // If the index is a constant
    if (I->GetIndex()->IsConstant()) {
      auto Index = ((Constant*)I->GetIndex())->GetIntValue();
      if (!SourceType.IsStruct())
        ConstantIndexPart = (SourceType.CalcElemSize(0) * Index);
      else // its a struct and has to determine the offset other way
          ConstantIndexPart = SourceType.GetElemByteOffset(Index);

      // If there is nothing to add, then exit now
      if (ConstantIndexPart == 0 && !GoalInstr.IsInvalid())
        return GoalInstr;

      // rather then issue an addition, it more effective to set the
      // StackAccess operand's offset to the index value
      if (IsStack) {
        GoalInstr.GetOperands()[1].SetOffset(ConstantIndexPart);
        return GoalInstr;
      }
    }
    // If the index resides in a register
    else {
      IndexIsInReg = true;
      if (!SourceType.IsStruct()) {
        if (!GoalInstr.IsInvalid())
          BB->InsertInstr(GoalInstr);

        auto Multiplier = SourceType.CalcElemSize(0);

        // edge case, identity: x * 1 = x
        // in this case only do a MOV or SEXT rather then MUL
        if (Multiplier == 1) {
          MULResVReg = ParentFunction->GetNextAvailableVReg();
          auto MOV = MachineInstruction(MachineInstruction::MOV, BB);
          MOV.AddVirtualRegister(MULResVReg, TM->GetPointerSize());
          MOV.AddOperand(IndexReg);

          // if sign extension needed, then swap the mov to that
          if (IndexReg.GetSize() < TM->GetPointerSize())
            MOV.SetOpcode(MachineInstruction::SEXT);
          BB->InsertInstr(MOV);
        }
        // general case
        // MOV the multiplier into a register
        // FIXME: this should not needed, only done because AArch64 does not
        // support immediate operands for MUL, this should be handled by the
        // target legalizer
        else {
          auto ImmediateVReg = ParentFunction->GetNextAvailableVReg();
          auto MOV = MachineInstruction(MachineInstruction::MOV, BB);
          MOV.AddVirtualRegister(ImmediateVReg, TM->GetPointerSize());
          MOV.AddImmediate(Multiplier);
          BB->InsertInstr(MOV);

          // if sign extension needed, then insert a sign extending first
          MachineInstruction SEXT;
          unsigned SEXTResVReg = 0;
          if (IndexReg.GetSize() < TM->GetPointerSize()) {
            SEXTResVReg = ParentFunction->GetNextAvailableVReg();
            SEXT = MachineInstruction(MachineInstruction::SEXT, BB);
            SEXT.AddVirtualRegister(SEXTResVReg, TM->GetPointerSize());
            SEXT.AddOperand(IndexReg);
            BB->InsertInstr(SEXT);
          }

          MULResVReg = ParentFunction->GetNextAvailableVReg();
          auto MUL = MachineInstruction(MachineInstruction::MUL, BB);
          MUL.AddVirtualRegister(MULResVReg, TM->GetPointerSize());
          // if sign extension did not happened, then jus use the IndexReg
          if (SEXT.IsInvalid())
            MUL.AddOperand(IndexReg);
          else // otherwise the result register of the SEXT operaton
            MUL.AddVirtualRegister(SEXTResVReg, TM->GetPointerSize());
          MUL.AddVirtualRegister(ImmediateVReg, TM->GetPointerSize());
          BB->InsertInstr(MUL);
        }
      }
      else // its a struct and has to determine the offset other way
        assert(!"TODO");
        //ConstantIndexPart = SourceType.GetElemByteOffset(Index);
    }

    if (!GoalInstr.IsInvalid() && !IndexIsInReg)
      BB->InsertInstr(GoalInstr);

    auto ADD = MachineInstruction(MachineInstruction::ADD, BB);
    ADD.AddOperand(Dest);
    // In case if the source is from a register (let say from a previous load)
    // then the second operand is simply this source reg
    if (IsReg)
      ADD.AddOperand(GetMachineOperandFromValue(I->GetSource(), BB));
    else
      // Otherwise (stack or global case) the base address is loaded in Dest by
      // the preceding STACK_ADDRESS or GLOBAL_ADDRESS instruction
      ADD.AddOperand(Dest);

    if (IndexIsInReg)
      ADD.AddVirtualRegister(MULResVReg, TM->GetPointerSize());
    else
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

    ResultMI.AddOperand(GetMachineOperandFromValue(I->GetCondition(), BB));
    ResultMI.AddLabel(LabelTrue);
    if (I->HasFalseLabel())
      ResultMI.AddLabel(LabelTrue);
  }
  // Compare instruction: cmp dest, src1, src2
  else if (auto I = dynamic_cast<CompareInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue((Value *)I, BB);
    auto FirstSrcOp = GetMachineOperandFromValue(I->GetLHS(), BB);
    auto SecondSrcOp = GetMachineOperandFromValue(I->GetRHS(), BB);

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

      // In case if its a struct by value param, then it is already loaded
      // in into registers, so issue move instructions to move these into
      // the parameter registers
      if (Param->GetTypeRef().IsStruct() && !Param->GetTypeRef().IsPTR()) {
        assert(StructByIDToRegMap.count(Param->GetID()) > 0 &&
               "The map does not know about this struct param");
        for (auto VReg : StructByIDToRegMap[Param->GetID()]) {
          Instr = MachineInstruction(MachineInstruction::MOV, BB);

          Instr.AddRegister(TargetArgRegs[ParamCounter]->GetID(),
                            TargetArgRegs[ParamCounter]->GetBitWidth());

          Instr.AddVirtualRegister(VReg, TM->GetPointerSize());
          BB->InsertInstr(Instr);
          ParamCounter++;
        }
      }
      // Handle pointer case for both local and global objects
      else if (Param->GetTypeRef().IsPTR() && (Param->IsGlobalVar() ||
          ParentFunction->IsStackSlot(Param->GetID()))) {
        unsigned DestinationReg;
        unsigned RegBitWidth = TargetArgRegs[ParamCounter]->GetBitWidth();

        if ((int)ParamCounter == I->GetImplicitStructArgIndex())
          DestinationReg = TM->GetRegInfo()->GetStructPtrRegister();
        else
          DestinationReg = TargetArgRegs[ParamCounter]->GetID();

        if (Param->IsGlobalVar()) {
          Instr = MachineInstruction(MachineInstruction::GLOBAL_ADDRESS, BB);

          Instr.AddRegister(DestinationReg, RegBitWidth);

          auto Symbol = ((GlobalVariable *)Param)->GetName();
          Instr.AddGlobalSymbol(Symbol);
          BB->InsertInstr(Instr);
          ParamCounter++;
        } else {
          Instr = MachineInstruction(MachineInstruction::STACK_ADDRESS, BB);

          Instr.AddRegister(DestinationReg, RegBitWidth);

          Instr.AddStackAccess(Param->GetID());
          BB->InsertInstr(Instr);
          ParamCounter++;
        }
      }
      // default case is to just move into the right parameter register
      else {
        Instr = MachineInstruction(MachineInstruction::MOV, BB);
        unsigned ParamIdx = ParamCounter;

        if (Param->IsFPType()) {
          Instr.SetOpcode(MachineInstruction::MOVF);
          ParamIdx += TM->GetABI()->GetFirstFPRetRegIdx();
        }

        auto Src = GetMachineOperandFromValue(Param, BB);
        auto ParamPhysReg = TargetArgRegs[ParamIdx]->GetID();
        auto ParamPhysRegSize = TargetArgRegs[ParamIdx]->GetBitWidth();

        if (Src.GetSize() < ParamPhysRegSize) {
          ParamPhysReg = TargetArgRegs[ParamIdx]->GetSubRegs()[0];
          ParamPhysRegSize =
              TM->GetRegInfo()
                  ->GetRegisterByID(
                      TargetArgRegs[ParamIdx]->GetSubRegs()[0])
                  ->GetBitWidth();
        }

        Instr.AddRegister(ParamPhysReg, ParamPhysRegSize);

        Instr.AddOperand(Src);
        BB->InsertInstr(Instr);
        ParamCounter++;
      }
    }

    ResultMI.AddFunctionName(I->GetName().c_str());

    // if no return value then we are done
    if (I->GetTypeRef().IsVoid())
      return ResultMI;

    /// Handle the case when there are returned values and spill them to the
    /// stack
    BB->InsertInstr(ResultMI);

    unsigned RetBitSize = I->GetTypeRef().GetByteSize() * 8;
    const unsigned MaxRegSize = TM->GetPointerSize();
    const unsigned RegsCount = GetNextAlignedValue(RetBitSize, MaxRegSize)
                               / MaxRegSize;
    assert(RegsCount > 0);
    auto &RetRegs = TM->GetABI()->GetReturnRegisters();

    for (size_t i = 0; i < RegsCount; i++) {
      // NOTE: Actual its not a vreg, but this make sure it will be a unique ID.
      // TODO: Maybe rename GetNextAvailableVReg to GetNextAvailableID
      auto StackSlot = ParentFunction->GetNextAvailableVReg();
      IRVregToLLIRVreg[I->GetID()] = StackSlot;
      SpilledReturnValuesStackIDs.insert(StackSlot);
      ParentFunction->InsertStackSlot(StackSlot,
                                      std::min(RetBitSize, MaxRegSize) / 8,
                                      std::min(RetBitSize, MaxRegSize) / 8);
      auto Store = MachineInstruction(MachineInstruction::STORE, BB);
      Store.AddStackAccess(StackSlot);

      // find the appropriate return register for the size
      unsigned TargetRetReg;

      size_t ParamIdx = i;
      if (I->GetTypeRef().IsFP())
        ParamIdx += TM->GetABI()->GetFirstFPRetRegIdx();

      // if the return value can use the return register
      if (std::min(RetBitSize, MaxRegSize) >= TM->GetPointerSize())
        TargetRetReg = RetRegs[ParamIdx]->GetID();
      // need to find an appropriate sized subregister of the actual return reg
      else
        // TODO: Temporary solution, only work for AArch64
        TargetRetReg = RetRegs[ParamIdx]->GetSubRegs()[0];

      Store.AddRegister(TargetRetReg, std::min(RetBitSize, MaxRegSize));
      // TODO: ...
      if (i + 1 == RegsCount)
        return Store;
      BB->InsertInstr(Store);
      RetBitSize -= MaxRegSize;
    }
  }
  // Ret instruction: ret op
  else if (auto I = dynamic_cast<ReturnInstruction *>(Instr); I != nullptr) {
    // If return is void
    if (I->GetRetVal() == nullptr)
      return ResultMI;

    // insert load to load in the return val to the return registers
    auto &TargetRetRegs = TM->GetABI()->GetReturnRegisters();
    if (I->GetRetVal()->GetTypeRef().IsStruct()) {
      // how many register are used to pass this struct
      unsigned StructBitSize =
          (I->GetRetVal()->GetTypeRef().GetBaseTypeByteSize() * 8);
      unsigned MaxRegSize = TM->GetPointerSize();
      unsigned RegsCount =
          GetNextAlignedValue(StructBitSize, MaxRegSize) / MaxRegSize;

      for (size_t i = 0; i < RegsCount; i++) {
        auto Instr = MachineInstruction(MachineInstruction::LOAD, BB);
        Instr.AddRegister(TargetRetRegs[i]->GetID(),
                          TargetRetRegs[i]->GetBitWidth());
        auto RetId = GetIDFromValue(I->GetRetVal());
        Instr.AddStackAccess(RetId, i * (TM->GetPointerSize() / 8));
        BB->InsertInstr(Instr);
      }
    }
    else if (I->GetRetVal()->IsConstant()) {
      auto &RetRegs = TM->GetABI()->GetReturnRegisters();

      auto LoadImm = MachineInstruction(MachineInstruction::LOAD_IMM, BB);

      // TODO: this assumes aarch64, make it target independent by searching
      // for the right sized register, like in the register allocator
      if (RetRegs[0]->GetBitWidth() == I->GetBitWidth())
        LoadImm.AddRegister(RetRegs[0]->GetID(), RetRegs[0]->GetBitWidth());
      else
        LoadImm.AddRegister(RetRegs[0]->GetSubRegs()[0],
                            TM->GetRegInfo()
                                ->GetRegisterByID(RetRegs[0]->GetSubRegs()[0])
                                ->GetBitWidth());
                  
      LoadImm.AddOperand(GetMachineOperandFromValue(I->GetRetVal(), BB));
      // change ret operand to the destination register of the LOAD_IMM
      ResultMI.AddOperand(*LoadImm.GetOperand(0));
      BB->InsertInstr(LoadImm);
    } else {
      auto Result = GetMachineOperandFromValue(I->GetRetVal(), BB);
      ResultMI.AddOperand(Result);
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
      Load.AddVirtualRegister(NewVReg, /* TODO: use alignment here */ 32);
      auto SrcId = GetIDFromValue(I->GetSource());

      if (ParentFunction->IsStackSlot(SrcId))
        Load.AddStackAccess(SrcId, i * /* TODO: use alignment here */ 4);
      else
        Load.AddMemory(SrcId, i * /* TODO: use alignment here */ 4, TM->GetPointerSize());

      BB->InsertInstr(Load);

      auto Store = MachineInstruction(MachineInstruction::STORE, BB);
      auto DestId = GetIDFromValue(I->GetDestination());
      if (ParentFunction->IsStackSlot(DestId))
        Store.AddStackAccess(DestId,i * /* TODO: use alignment here */ 4);
      else {
        Store.AddMemory(DestId, TM->GetPointerSize());
        Store.GetOperands()[0].SetOffset(i * /* TODO: use alignment here */ 4);
      }
      Store.AddVirtualRegister(NewVReg, /* TODO: use alignment here */ 32);
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
                           MachineFunction *Func, TargetMachine *TM) {
  auto ReferredType = Instr->GetType();
  assert(ReferredType.GetPointerLevel() > 0);
  ReferredType.DecrementPointerLevel();
  auto IsPTR = ReferredType.GetPointerLevel() > 0;
  Func->InsertStackSlot(Instr->GetID(), IsPTR ? TM->GetPointerSize() / 8 :
                                              ReferredType.GetByteSize(),
                        ReferredType.GetBaseTypeByteSize());
}

void IRtoLLIR::HandleFunctionParams(Function &F, MachineFunction *Func) {
  for (auto &Param : F.GetParameters()) {
    auto ParamID = Param->GetID();
    auto ParamSize = Param->GetBitWidth();
    auto IsStructPtr = Param->IsImplicitStructPtr();

    // Handle structs
    if (Param->GetTypeRef().IsStruct() && !Param->GetTypeRef().IsPTR()) {
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
        auto NextVReg = Func->GetNextAvailableVReg();
        StructToRegMap[StructName].push_back(NextVReg);
        Func->InsertParameter(NextVReg,
                              LowLevelType::CreateScalar(TM->GetPointerSize()));
      }

      continue;
    }

    if (Param->GetTypeRef().IsPTR())
      Func->InsertParameter(
          ParamID, LowLevelType::CreatePTR(TM->GetPointerSize()), IsStructPtr);
    else
      Func->InsertParameter(ParamID, LowLevelType::CreateScalar(ParamSize),
                            IsStructPtr, Param->GetTypeRef().IsFP());
  }
}

void IRtoLLIR::GenerateLLIRFromIR() {
  // reserving enough size for the functions otherwise the underlying vector
  // would reallocate it self and would made invalid the existing pointers
  // pointing to these functions
  // FIXME: Would be nice to auto update the pointers somehow if necessary.
  // Like LLVM does it, but that might be too complicated for the scope of this
  // project.
  TU->GetFunctions().reserve(IRM.GetFunctions().size());

  for (auto &Fun : IRM.GetFunctions()) {
    // reset state
    Reset();

    // function declarations does not need any LLIR code
    if (Fun.IsDeclarationOnly())
      continue;

    TU->AddNewFunction();
    MachineFunction *MFunction = TU->GetCurrentFunction();
    assert(MFunction);

    MFunction->SetName(Fun.GetName());
    HandleFunctionParams(Fun, MFunction);

    // Create all basic block first with their name, so jumps can refer to them
    // already
    auto &MFuncMBBs = MFunction->GetBasicBlocks();
    for (auto &BB : Fun.GetBasicBlocks())
      MFuncMBBs.push_back(MachineBasicBlock{BB.get()->GetName(), MFunction});

    unsigned BBCounter = 0;
    for (auto &BB : Fun.GetBasicBlocks()) {
      for (auto &Instr : BB->GetInstructions()) {
        auto InstrPtr = Instr.get();

        if (InstrPtr->IsStackAllocation()) {
          HandleStackAllocation((StackAllocationInstruction *)InstrPtr,
                                MFunction, TM);
          continue;
        }
        MFuncMBBs[BBCounter].InsertInstr(
            ConvertToMachineInstr(InstrPtr, &MFuncMBBs[BBCounter], MFuncMBBs));

        // everything after a return is dead code so skip those
        // TODO: add unconditional branch aswell, but it would be better to
        // just not handle this here but in some optimization pass for
        // example in dead code elimination
        if (MFuncMBBs[BBCounter].GetInstructions().back().IsReturn())
          break;
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
      if (InitList.empty()) {
        auto InitStr = ((GlobalVariable *)GlobalVar.get())->GetInitString();
        auto InitVal = ((GlobalVariable *)GlobalVar.get())->GetInitValue();

        if (InitStr.empty() && InitVal == nullptr)
          GD.InsertAllocation(Size, 0);
        else // string literal case
          GD.InsertAllocation(InitVal ? ((GlobalVariable *)InitVal)->GetName()
                                      : InitStr);
      }
      // if the list is not empty then allocate the appropriate type of
      // memories with initialization
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
    else if (InitList.empty()) {
      auto InitVal = ((GlobalVariable *)GlobalVar.get())->GetInitValue();

      // zero initalized scalar
      if (InitVal == nullptr)
        GD.InsertAllocation(Size, 0);
      // initialized by another global variable (ptr), typical use case: string
      // literals
      else {
        GlobalData::Directives d;

        switch (TM->GetPointerSize()) {
        case 32:
          d = GlobalData::WORD;
          break;
        case 64:
          d = GlobalData::DOUBLE_WORD;
          break;
        default:
          assert(!"Unhandled pointer size");
          break;
        }

        GD.InsertAllocation(((GlobalVariable *)InitVal)->GetName(), d);
      }
    } else {
      GD.InsertAllocation(Size, InitList[0]);
    }

    TU->AddGlobalData(GD);
  }
}
