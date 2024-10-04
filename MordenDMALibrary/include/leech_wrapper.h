#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "vmmdll.h"

template <typename T>
class HandleWrapper {
 public:
  HandleWrapper(T* handle) : handle_(handle, nullptr) {}
  HandleWrapper(T* handle, std::function<void(T*)> deleter)
      : handle_(handle, deleter) {}

  void reset(T* handle) { handle_.reset(handle); }

  T* get() const { return handle_.get(); }

  operator T*() const { return handle_.get(); }

 private:
  std::unique_ptr<T, std::function<void(T*)>> handle_;
};

namespace LC {
void HandleDeleter(HANDLE handle);
};
namespace VMM {

const int32_t DEFAULT_PAGE_BYTES = 4096;

void HandleDeleter(VMM_HANDLE handle);

VMM_HANDLE Initialize(const std::string& arguments);

bool ConfigGet(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
               uint64_t, uint64_t&);
bool ConfigSet(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
               uint64_t, uint64_t);

std::optional<std::vector<uint8_t>> MemReadPage(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr);

std::optional<std::vector<uint8_t>> MemReadPage(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr);

bool MemPrefetchPages(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
                      const uint32_t pid,
                      std::vector<uint64_t> prefetch_addresses);

std::optional<uint64_t> MemVirt2Phys(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr);

BOOL VMMDLL_MemSearch(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID,
                      _Inout_ PVMMDLL_MEM_SEARCH_CONTEXT ctx,
                      _Out_ PQWORD* ppva, _Out_ PDWORD pcva);

std::optional<std::vector<uint8_t>> MemReadEx(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr, const std::size_t bytes,
    const uint32_t flag);

bool MemWrite(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
              const uint32_t pid, const uint64_t addr,
              std::vector<uint8_t>& data);

uint32_t MemReadScatter(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> hVMM,
                        const uint32_t dwPID, PPMEM_SCATTER ppMEMs,
                        int32_t cpMEMs, int32_t flags);
uint32_t MemWriteScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> hVMM,
    const uint32_t dwPID, PPMEM_SCATTER ppMEMs, int32_t cpMEMs);

struct MemorySearchContext {
  std::vector<VMMDLL_MEM_SEARCH_CONTEXT_SEARCHENTRY> search_entry{3};
  std::vector<std::vector<uint8_t>> pattern_buffer{3, {32}};
  VMMDLL_MEM_SEARCH_CONTEXT raw_context{.dwVersion = VMMDLL_MEM_SEARCH_VERSION};

  MemorySearchContext(std::vector<std::vector<uint8_t>>& targets,
                      uint64_t min_virtual_address = 0,
                      uint64_t read_flags = 0) {
    search_entry.resize(targets.size());
    raw_context.pSearch = search_entry.data();
    for (uint32_t entry_num = 0; entry_num < targets.size(); entry_num++) {
      search_entry.at(entry_num).cb = targets.at(entry_num).size();

      std::memcpy(search_entry.at(entry_num).pb,
                  pattern_buffer.at(entry_num).data(),
                  pattern_buffer.at(entry_num).size());

      search_entry.at(entry_num).cbAlign = 0x1000;
    }

    raw_context.vaMin = min_virtual_address;
    raw_context.ReadFlags = read_flags;
  };
};

struct ScatterRequestPackage {
  MEM_SCATTER scatter{
      .version = MEM_SCATTER_VERSION, .f = false, .cb = DEFAULT_PAGE_BYTES};
  int64_t address;
  int32_t length = DEFAULT_PAGE_BYTES;
  std::vector<uint8_t> buffer;
};
using SRP = ScatterRequestPackage;

class Scatter {
 public:
  Scatter(uint32_t pid, uint32_t flags);

  void AddSRP(const SRP& srp);
  void AddSRP(const std::vector<SRP>& srps);
  void AddSRP(int64_t address, int32_t length);
  bool RemoveSRP(const SRP& srp);
  bool RemoveSRP(int64_t address);
  SRP* GetSRP(int64_t address);
  std::vector<uint8_t>& GetData(int64_t address);

  const std::map<int64_t, SRP>& SRPMap() noexcept;

  bool ExecuteRead();
  bool ExecuteWrite();

  void SetPID(uint32_t pid);
  void SetFlags(uint32_t flags);

 private:
  std::map<int64_t, SRP> srp_map_;
  uint32_t pid_;
  uint32_t flags_;
  std::chrono::system_clock::time_point last_execution_;
};
};  // namespace VMM

#endif
