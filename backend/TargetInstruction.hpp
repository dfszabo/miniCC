#ifndef TARGET_INSTRUCTION_HPP
#define TARGET_INSTRUCTION_HPP

#include <string>
#include <vector>

class TargetInstruction {
public:
  enum Attributes : unsigned {
    LOAD = 1,
    STORE = 1 << 1,
    RETURN = 1 << 2,
  };

  TargetInstruction() {}
  TargetInstruction(unsigned OperationID, unsigned Size, const char *AsmString,
                    std::vector<unsigned> OperandTypes)
      : OperationID(OperationID), Size(Size), AsmString(AsmString),
        OperandTypes(OperandTypes) {}

  TargetInstruction(unsigned OperationID, unsigned Size, const char *AsmString,
                    std::vector<unsigned> OperandTypes, unsigned Attributes)
      : OperationID(OperationID), Size(Size), AsmString(AsmString),
        OperandTypes(OperandTypes), Attributes(Attributes) {}

  std::string &GetAsmString() { return AsmString; }

  unsigned GetOperationID() const { return OperationID; }
  unsigned GetOperandNumber() const { return OperandTypes.size(); }

  unsigned GetSize() const { return Size; }

  bool IsLoad() const { return (Attributes & LOAD) != 0; }
  bool IsStore() const { return (Attributes & STORE) != 0; }
  bool IsReturn() const { return (Attributes & RETURN) != 0; }
  bool IsLoadOrStore() const { return IsLoad() || IsStore(); }

private:
  unsigned OperationID;
  unsigned Size;
  std::string AsmString;
  std::vector<unsigned> OperandTypes;
  unsigned Attributes = 0;
};

#endif
