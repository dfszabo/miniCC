#include "Value.hpp"

uint64_t Constant::GetIntValue() const {
  assert(ValueType.IsINT());
  int64_t result = 0;
  uint64_t val = std::get<uint64_t>(Val);

  // sign extend it
  // TODO: maybe Val should be stored as int64_t not uint64_t
  if (GetBitWidth() == 8)
    result = (int8_t)val;
  else if (GetBitWidth() == 16)
    result = (int16_t)val;
  else if (GetBitWidth() == 32)
    result = (int32_t)val;
  else
    result = val;

  return result;
}

double Constant::GetFloatValue() const {
  assert(ValueType.IsFP());
  double result = std::get<double>(Val);

  assert(GetBitWidth() == 64 && "Only doubles supported");

  return result;
}

std::string Constant::ValueString() const {
  std::string str;

  if (IsFPConst())
    str += std::to_string(std::get<double>(Val));
  else
    str += std::to_string((int64_t)GetIntValue());

  str += "<" + ValueType.AsString() + ">";
  return str;
}

void GlobalVariable::Print() const {
  std::cout << "global var (" << GetType().AsString() << "):" << std::endl
            << "\t" << Name;

  if (!InitList.empty()) {
    std::cout << " = {";
    for (size_t i = 0; i < InitList.size(); i++) {
      std::cout << " " << std::to_string(InitList[i]);
      if (i  + 1 < InitList.size())
        std::cout << ",";
    }
    std::cout << " }";
  } else if (!InitString.empty()) {
    std::cout << " = \"" << InitString << "\"";
  } else if (auto GV = dynamic_cast<GlobalVariable *>(InitValue);
             GV != nullptr) {
    std::cout << " = " << GV->GetName();
  }

  std::cout << std::endl << std::endl;
}
