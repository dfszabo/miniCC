#include "StackFrame.hpp"
#include "Support.hpp"
#include <cassert>
#include <iostream>

void StackFrame::InsertStackSlot(unsigned ID, unsigned Size, unsigned Align) {
  assert(StackSlots.count(ID) == 0 && "Already existing object on the stack");

  ObjectsSize = GetNextAlignedValue(ObjectsSize, Align);
  ObjectsSize += Size;
  
  StackSlots.insert({ID, {Size, Align}});
}

unsigned StackFrame::GetPosition(unsigned ID) {
  assert(IsStackSlot(ID) && "Must be a valid stack slot ID");

  unsigned Position = 0;

  for (const auto &[ObjectID, ObjectSizeAndAlign] : StackSlots) {
    const auto [ObjectSize, ObjectAlign] = ObjectSizeAndAlign;

    // If the stack object is what we are looking for
    if (ObjectID == ID)
      // then return its aligned position
      return GetNextAlignedValue(Position, ObjectAlign);

    Position = GetNextAlignedValue(Position, ObjectAlign);
    Position += ObjectSize;
  }

  return ~0; // Error
}

unsigned StackFrame::GetSize(unsigned ID) {
  assert(IsStackSlot(ID) && "Must be a valid stack slot ID");

  return StackSlots[ID].first;
}

void StackFrame::Print() const {
  unsigned Number = 0;
  std::cout << "\t\tFrameSize: " << ObjectsSize << std::endl;

  for (const auto &FrameObj : StackSlots)
    std::cout << "\t\tPosition: " << Number++ << ", ID: " << FrameObj.first
              << ", Size: " << FrameObj.second.first
              << ", Align: " << FrameObj.second.second << std::endl;

  std::cout << std::endl;
}
