cmake_minimum_required(VERSION 3.10)
project(mini-c-compiler VERSION 0.0.1)

set(CMAKE_BUILD_TYPE "Debug")

# For debug purposes uncomment this
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wno-reorder")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

add_executable(miniCC
    frontend/frontend_test.cpp
    frontend/preprocessor/PPLexer.cpp
    frontend/preprocessor/PreProcessor.cpp
    frontend/parser/Parser.cpp
    frontend/lexer/Lexer.cpp
    frontend/ast/AST.cpp
    middle_end/IR/BasicBlock.cpp
    middle_end/IR/Function.cpp
    middle_end/IR/Instructions.cpp
    middle_end/IR/Module.cpp
    middle_end/IR/Type.cpp
    backend/AssemblyEmitter.cpp
    backend/IRtoLLIR.cpp
    backend/InstructionSelection.cpp
    backend/MachineBasicBlock.cpp
    backend/MachineFunction.cpp
    backend/MachineInstruction.cpp
    backend/MachineInstructionLegalizer.cpp
    backend/MachineOperand.cpp
    backend/PrologueEpilogInsertion.cpp
    backend/RegisterAllocator.cpp
    backend/Support.cpp
    backend/StackFrame.cpp
    backend/TargetInstructionLegalizer.cpp
    backend/TargetMachine.cpp
    backend/TargetArchs/AArch64/AArch64RegisterInfo.cpp
    backend/TargetArchs/AArch64/AArch64InstructionDefinitions.cpp
    backend/TargetArchs/AArch64/AArch64InstructionLegalizer.cpp
    backend/TargetArchs/AArch64/AArch64TargetABI.cpp
    backend/TargetArchs/AArch64/AArch64TargetMachine.cpp
    backend/TargetArchs/RISCV/RISCVRegisterInfo.cpp
    backend/TargetArchs/RISCV/RISCVInstructionDefinitions.cpp
    backend/TargetArchs/RISCV/RISCVTargetABI.cpp
    backend/TargetArchs/RISCV/RISCVTargetMachine.cpp)
