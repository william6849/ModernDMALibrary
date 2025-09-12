#include "scatter.h"

Scatter::Scatter(uint32_t pid, uint32_t flags = VMMDLL_FLAG_NOCACHE |
                                                VMMDLL_FLAG_ZEROPAD_ON_FAIL |
                                                VMMDLL_FLAG_NOPAGING)
    : pid_(pid), flags_(flags) {}

void Scatter::AddSRP(const SRP& srp) {
  srp_map_[srp.address] = srp;
  auto& entry = srp_map_[srp.address];
  entry.buffer.resize(entry.length);
  entry.scatter.qwA = entry.address;
  entry.scatter.cb = entry.length;
  entry.scatter.pb = entry.buffer.data();
}

void Scatter::AddSRP(const std::vector<SRP>& srps) {
  for (auto& srp : srps) {
    AddSRP(srp);
  }
}

void Scatter::AddSRP(uint64_t address, uint32_t length) {
  SRP srp = {.address = address, .length = length};
  AddSRP(std::move(srp));
}

bool Scatter::RemoveSRP(const SRP& srp) { return RemoveSRP(srp.address); }

bool Scatter::RemoveSRP(uint64_t address) {
  auto it = srp_map_.find(address);
  if (it == srp_map_.end()) {
    return false;
  }
  srp_map_.erase(it);
  return true;
}

SRP* Scatter::GetSRP(uint64_t address) {
  auto it = srp_map_.find(address);
  if (it == srp_map_.end()) {
    return nullptr;
  }
  return &(it->second);
}

std::vector<uint8_t>& Scatter::GetData(uint64_t address) {
  auto it = srp_map_.find(address);
  if (it == srp_map_.end()) {
    throw std::runtime_error("SRP not exist.");
  }
  return it->second.buffer;
}

std::map<uint64_t, SRP>& Scatter::SRPMap() noexcept { return srp_map_; }

const uint32_t Scatter::pid() const { return pid_; }

const uint32_t Scatter::flags() const { return flags_; }

void Scatter::SetPID(uint32_t pid) { pid_ = pid; }

void Scatter::SetFlags(uint32_t flags) { flags_ = flags; }