#ifndef FRONTEND_SUPPORT_H
#define FRONTEND_SUPPORT_H

template <typename Base, typename T> inline bool instanceof (const T *ptr) {
  return dynamic_cast<const Base *>(ptr) != nullptr;
}

#endif
