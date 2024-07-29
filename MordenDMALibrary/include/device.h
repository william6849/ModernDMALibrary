#ifndef DEVICE_H
#define DEVICE_H

#include <string>

#include "memory.h"

class Device {
 public:
  static Device& GetInstance();
  Device(const Device&) = delete;
  void operator=(const Device&) = delete;
  // virtual ~Device();

  static bool InitDevice(std::string device_path);

 private:
  Device();
  MemoryControl memctl;
};

#endif