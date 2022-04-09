#include "AArch64XRegToWRegFixPass.hpp"
#include "../../MachineBasicBlock.hpp"
#include "../../MachineFunction.hpp"
#include "../../MachineInstruction.hpp"
#include "AArch64InstructionDefinitions.hpp"

void AArch64XRegToWRegFixPass::Run() {
  for (auto &MFunc : MIRM->GetFunctions())
    for (auto &MBB : MFunc.GetBasicBlocks())
      for (auto &Instr : MBB.GetInstructions())
        if ((Instr.GetOpcode() == AArch64::MOV_rr ||
             Instr.GetOpcode() == AArch64::AND_rri) &&
            Instr.GetOperand(0)->GetSize() == 32 &&
            Instr.GetOperand(1)->GetSize() == 64) {
          auto SrcXReg = Instr.GetOperand(1)->GetReg();
          assert(!TM->GetRegInfo()->GetRegisterByID(SrcXReg)->GetSubRegs().empty());
          auto WReg =
              TM->GetRegInfo()->GetRegisterByID(SrcXReg)->GetSubRegs()[0];
          Instr.GetOperand(1)->SetReg(WReg);
        }
}
