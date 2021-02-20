#include "StackFrame.hpp"
#include <cassert>

void StackFrame::InsertStackSlot(unsigned ID, unsigned Size) {
  assert(StackSlots.count(ID) == 0 && "Already existing object on the stack");
  ObjectsSize += Size;
  StackSlots.insert({ID, Size});
}

unsigned StackFrame::GetPosition(unsigned ID) {
  assert(IsStackSlot(ID) && "Must be a valid stack slot ID");

  unsigned Position = 0;

  for (const auto &Entry : StackSlots) {
    // If the stack object is what we are looking for
    if (Entry.first == ID)
      return Position; // then return its position

    // FIXME: See InsertStackSlot comment
    Position += Entry.second;
  }

  return ~0; // Error
}