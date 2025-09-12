#pragma once

namespace VMM {

std::expected<VMM_HANDLE, std::string> Initialize(std::string_view arguments);

};