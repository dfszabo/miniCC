#include "Value.hpp"


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
