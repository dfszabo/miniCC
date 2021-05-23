#include "StackFrame.hpp"
#include <cassert>
#include <iostream>

void StackFrame::InsertStackSlot(unsigned ID, unsigned Size) {
  assert(StackSlots.count(ID) == 0 && "Already existing object on the stack");

  if (Size >= 4)
    ObjectsSize += Size;
  else
    ObjectsSize += 4;
  
  StackSlots.insert({ID, Size});
}

unsigned StackFrame::GetPosition(unsigned ID) {
  assert(IsStackSlot(ID) && "Must be a valid stack slot ID");

  unsigned Position = 0;

  for (const auto &[ObjectID, ObjectSize] : StackSlots) {
    // If the stack object is what we are looking for
    if (ObjectID == ID)
      return Position; // then return its position

    // NOTE: Hard coded 4 byte alignment
    if (ObjectSize >= 4)
      Position += ObjectSize;
    else
      Position += 4;
  }

  return ~0; // Error
}

unsigned StackFrame::GetSize(unsigned ID) {
  assert(IsStackSlot(ID) && "Must be a valid stack slot ID");

  return StackSlots[ID];
}

void StackFrame::Print() const {
  unsigned Number = 0;
  std::cout << "\t\tFrameSize: " << ObjectsSize << std::endl;

  for (const auto &FrameObj : StackSlots)
    std::cout << "\t\tPosition: " << Number++ << ", ID: " << FrameObj.first
              << ", Size: " << FrameObj.second << std::endl;

  std::cout << std::endl;
}
