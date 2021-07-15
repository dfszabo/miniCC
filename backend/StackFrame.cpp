#include "StackFrame.hpp"
#include "Support.hpp"
#include <cassert>
#include <iostream>

void StackFrame::InsertStackSlot(unsigned ID, unsigned Size) {
  assert(StackSlots.count(ID) == 0 && "Already existing object on the stack");

  ObjectsSize = GetNextAlignedValue(ObjectsSize, Size);
  ObjectsSize += Size;
  
  StackSlots.insert({ID, Size});
}

unsigned StackFrame::GetPosition(unsigned ID) {
  assert(IsStackSlot(ID) && "Must be a valid stack slot ID");

  unsigned Position = 0;

  // For now the object size is its alignment as well
  // FIXME: add also alignment info
  for (const auto &[ObjectID, ObjectSize] : StackSlots) {
    // If the stack object is what we are looking for
    if (ObjectID == ID)
      // then return its aligned position
      return GetNextAlignedValue(Position, ObjectSize);

    Position = GetNextAlignedValue(Position, ObjectSize);
    Position += ObjectSize;
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
