#ifndef IR_UTIL_HPP
#define IR_UTIL_HPP

#include "../IR/BasicBlock.hpp"
#include <map>

/// Containing helper functions, which has uses in multiple
/// locations.

/// Map registers (which Values) in the instruction list based on
/// @Renameable mapping. If an instruction in @InstrList uses a Value,
/// which has mapping in @Renamables, then change the use the the mapped Value.
void RenameRegisters(std::map<Value *, Value *> &Renameables,
                     BasicBlock::InstructionList &InstrList);

#endif // IR_UTIL_HPP
