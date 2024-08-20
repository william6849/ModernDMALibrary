#include "device.h"

#include <stdexcept>

Device::Device(std::string path) : vmm_handle_(nullptr) {
  if (path.find("-fpga") || path.find("")) {
    LPCSTR args[] = {(LPCSTR) "", (LPCSTR) "-device", (LPCSTR) "fpga",
                     (LPCSTR) "-memmap", (LPCSTR) "/home/zznzm/repos/MordenDMALibrary/build/unixlike-gcc-release/"
      "third_party/Source/MemProcFS/dump.txt"};
    auto hVMM = VMMDLL_Initialize(5, args);
    if (hVMM) {
      vmm_handle_.reset(hVMM);
    } else {
      throw std::runtime_error(
          "VMM operation failed(memo to make custom VMM exceptions)");
    }
  } else {
    throw std::invalid_argument("received invalid value");
  }
};
