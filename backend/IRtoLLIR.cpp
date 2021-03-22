#include "IRtoLLIR.hpp"
#include "../middle_end/IR/BasicBlock.hpp"
#include "../middle_end/IR/Function.hpp"
#include "LowLevelType.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include <cassert>

MachineOperand GetMachineOperandFromValue(Value *Val) {
  if (Val->IsRegister()) {
    return MachineOperand::CreateVirtualRegister(Val->GetID());
  } else if (Val->IsParameter()) {
    MachineOperand Result;
    Result.SetToParameter();
    // FIXME: Only handling int params now, handle others too
    // And add type to registers and others too
    Result.SetType(LowLevelType::CreateINT(32));
    Result.SetValue(Val->GetID());
    return Result;
  } else if (Val->IsConstant()) {
    auto C = dynamic_cast<Constant *>(Val);
    assert(!C->IsFPConst() && "TODO");
    return MachineOperand::CreateImmediate(C->GetIntValue());
  } else {
    assert(!"Unhandled MO case");
  }

  return MachineOperand();
}

MachineInstruction ConverToMachineInstr(Instruction *Instr,
                                        MachineBasicBlock *BB,
                                        std::vector<MachineBasicBlock> &BBs) {
  auto Operation = Instr->GetInstructionKind();
  auto ParentFunction = BB->GetParent();

  auto ResultMI = MachineInstruction((unsigned)Operation + (1 << 16), BB);

  // Three address ALU instructions: INSTR Result, Op1, Op2
  if (auto I = dynamic_cast<BinaryInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue((Value *)I);
    auto FirstSrcOp = GetMachineOperandFromValue(I->GetLHS());
    auto SecondSrcOp = GetMachineOperandFromValue(I->GetRHS());

    ResultMI.AddOperand(Result);
    ResultMI.AddOperand(FirstSrcOp);
    ResultMI.AddOperand(SecondSrcOp);
  }
  // Store instruction: STR [address], Src
  else if (auto I = dynamic_cast<StoreInstruction *>(Instr); I != nullptr) {
    // FIXME: maybe it should be something else then a register since its
    // an address, revisit this
    assert(I->GetMemoryLocation()->IsRegister() && "Must be a register");

    ResultMI.AddAttribute(MachineInstruction::IS_STORE);

    auto AddressReg = I->GetMemoryLocation()->GetID();

    // Check if the instruction accessing the stack
    if (ParentFunction->IsStackSlot(AddressReg))
      // if it is then set the operand to a stack access
      ResultMI.AddStackAccess(AddressReg);
    else // otherwise a normal memory access
      ResultMI.AddMemory(AddressReg);

    ResultMI.AddOperand(GetMachineOperandFromValue(I->GetSavedValue()));
  }
  // Load instruction: LD Dest, [address]
  else if (auto I = dynamic_cast<LoadInstruction *>(Instr); I != nullptr) {
    // FIXME: same as with STORE
    assert(I->GetMemoryLocation()->IsRegister() && "Must be a register");

    ResultMI.AddAttribute(MachineInstruction::IS_LOAD);

    ResultMI.AddOperand(GetMachineOperandFromValue((Value *)I));
    auto AddressReg = I->GetMemoryLocation()->GetID();

    // Check if the instruction accessing the stack
    if (ParentFunction->IsStackSlot(AddressReg))
      // if it is then set the operand to a stack access
      ResultMI.AddStackAccess(AddressReg);
    else // otherwise a normal memory access
      ResultMI.AddMemory(AddressReg);
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

    ResultMI.AddOperand(GetMachineOperandFromValue(I->GetCondition()));
    ResultMI.AddLabel(LabelTrue);
    if (I->HasFalseLabel())
      ResultMI.AddLabel(LabelTrue);
  }
  // Compare instruction: ret op
  else if (auto I = dynamic_cast<CompareInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue((Value *)I);
    auto FirstSrcOp = GetMachineOperandFromValue(I->GetLHS());
    auto SecondSrcOp = GetMachineOperandFromValue(I->GetRHS());

    ResultMI.AddOperand(Result);
    ResultMI.AddOperand(FirstSrcOp);
    ResultMI.AddOperand(SecondSrcOp);

    ResultMI.SetAttributes(I->GetRelation());
  }
  // Ret instruction: ret op
  else if (auto I = dynamic_cast<ReturnInstruction *>(Instr); I != nullptr) {
    auto Result = GetMachineOperandFromValue(I->GetRetVal());
    ResultMI.AddOperand(Result);
  } else
    assert(!"Unimplemented instruction!");

  return ResultMI;
}

/// For each stack allocation instruction insert a new entry into the StackFrame
void HandleStackAllocation(StackAllocationInstruction *Instr,
                           MachineFunction *Func) {
  Func->InsertStackSlot(Instr->GetID(), Instr->GetType().GetByteSize());
}

void HandleFunctionParams(Function &F, MachineFunction *Func) {
  for (auto &Param : F.GetParameters()) {
    auto ParamID = Param->GetID();
    assert(Param->IsIntType() && "Other types UNIMPLEMENTED YET");
    auto ParamSize = Param->GetBitWidth();

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
            ConverToMachineInstr(InstrPtr, &MFuncMBBs[BBCounter], MFuncMBBs));
      }

      BBCounter++;
    }
  }
}
