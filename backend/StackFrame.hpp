#ifndef STACKFRAME_HPP
#define STACKFRAME_HPP

#include <map>

class StackFrame {
  using StackSlotMap = std::map<unsigned, std::pair<unsigned, unsigned>>;

public:
  StackFrame() {}

  unsigned GetSize() const { return ObjectsSize; }
  unsigned GetEntriesCount() const { return StackSlots.size(); }

  void InsertStackSlot(unsigned ID, unsigned Size, unsigned Align);

  bool IsStackSlot(unsigned ID) const { return 0 != StackSlots.count(ID); }

  unsigned GetPosition(unsigned ID);
  unsigned GetSize(unsigned ID);

  void Print() const;

private:
  unsigned ObjectsSize = 0;

  // Maps an object ID to Its size and alignment. The order of the entries
  // represents the order of the objects pushed to the stack.
  StackSlotMap StackSlots;
};

#endif
