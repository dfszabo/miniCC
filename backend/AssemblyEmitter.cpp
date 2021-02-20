#include "AssemblyEmitter.hpp"
#include "TargetInstruction.hpp"
#include "TargetRegister.hpp"
#include <cassert>
#include <iostream>

void AssemblyEmitter::GenerateAssembly() {
  for (auto &Func : MIRM->GetFunctions()) {
    std::cout << ".globl\t" << Func.GetName() << std::endl;
    std::cout << Func.GetName() << ":" << std::endl;

    bool IsFirstBB = true;
    for (auto &BB : Func.GetBasicBlocks()) {
      if (!IsFirstBB) {
        std::cout << ".L" << BB.GetName() << ":" << std::endl;
      } else
        IsFirstBB = false;

      for (auto &Instr : BB.GetInstructions()) {
        std::cout << "\t";

        auto TargetInstr =
            TM->GetInstrDefs()->GetTargetInstr(Instr.GetOpcode());
        assert(TargetInstr != nullptr && "Something went wrong here");

        std::string AssemblyTemplateStr = TargetInstr->GetAsmString();
        const auto OperandNumber = TargetInstr->GetOperandNumber();

        // If the target instruction has no operands, then just print it and
        // continue
        if (OperandNumber == 0) {
          std::cout << AssemblyTemplateStr << std::endl;
          continue;
        }

        // Substitute the stringified operands to their appropriate places
        // example:
        // add $1, $2, $3 -> add a0, a1, a2
        for (size_t i = 0; i < OperandNumber; i++) {
          std::size_t DollarPos = AssemblyTemplateStr.find('$');

          if (DollarPos == std::string::npos)
            assert(!"The number of template operands are not match the"
                    "number of operands");

          unsigned NthOperand = AssemblyTemplateStr[DollarPos + 1] - '0';
          auto CurrentOperand = Instr.GetOperand(NthOperand - 1);

          // Register case
          if (CurrentOperand->IsRegister()) {
            TargetRegister *Reg =
                TM->GetRegInfo()->GetRegisterByID(CurrentOperand->GetReg());
            std::string RegStr;
            if (Reg->GetAlias() != "")
              RegStr = Reg->GetAlias();
            else
              RegStr = Reg->GetName();

            AssemblyTemplateStr.replace(DollarPos, 2, RegStr);
          }
          // Immediate case
          else if (CurrentOperand->IsImmediate()) {
            std::string ImmStr = std::to_string(CurrentOperand->GetImmediate());
            AssemblyTemplateStr.replace(DollarPos, 2, ImmStr);
          }
          // Label case
          else if (CurrentOperand->IsLabel()) {
            std::string Label = ".L";
            Label.append(CurrentOperand->GetLabel());
            AssemblyTemplateStr.replace(DollarPos, 2, Label);
          } else
            assert(!"Invalid Machine Operand type");
        }
        // Emit the final assembly string
        std::cout << AssemblyTemplateStr << std::endl;
      }
    }
  }
}
