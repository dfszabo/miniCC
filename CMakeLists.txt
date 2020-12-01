cmake_minimum_required(VERSION 3.10)
project(mini-c-compiler VERSION 0.0.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g")

add_executable(miniCC frontend/frontend_test.cpp
    frontend/parser/Parser.cpp
    frontend/lexer/Lexer.cpp
    frontend/ast/AST.cpp
    middle_end/IR/BasicBlock.cpp
    middle_end/IR/Function.cpp
    middle_end/IR/Instructions.cpp
    middle_end/IR/Module.cpp
    middle_end/IR/Type.cpp)
