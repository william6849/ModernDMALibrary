#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>

class MemoryControl {
 public:
  static MemoryControl& GetInstance();
  MemoryControl(const MemoryControl&) = delete;
  void operator=(const MemoryControl&) = delete;
  // virtual ~MemoryControl();

 private:
  MemoryControl();
  void InitDevice();
};

#endif  // MEMORY_H