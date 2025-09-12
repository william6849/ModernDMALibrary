#include "initialization.h"

namespace VMM {

std::expected<VMM_HANDLE, std::string> Initialize(std::string_view params) {
  spdlog::debug("Parameters: {}", params);

  if (params.empty()) {
    return std::unexpected("No parameters provided");
  }

  std::stringstream params_stream(std::string(params));
  auto params_parsed = std::vector<std::string>{};
  std::string token;
  while (getline(params_stream, token, ' ')) {
    if (!token.empty()) {
      params_parsed.push_back(token);
    }
  }

  if (params_parsed.empty()) {
    return std::unexpected("Failed to parse parameters");
  }

  auto c_str_vec = std::vector<LPCSTR>{};
  for (const auto& param : params_parsed) {
    c_str_vec.push_back(param.c_str());
  }

  VMM_HANDLE handle =
      VMMDLL_Initialize(static_cast<int>(c_str_vec.size()), c_str_vec.data());
  if (!handle) {
    std::string joined;
    for (const auto& p : params_parsed) {
      joined += p + " ";
    }
    return std::unexpected("VMMDLL_Initialize failed with params: " + joined);
  }

  return handle;
}
};  // namespace VMM