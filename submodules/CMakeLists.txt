# Google test module
init_git_submodule(${CMAKE_CURRENT_SOURCE_DIR}/googletest)
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)

# Spdlog
init_git_submodule(${CMAKE_CURRENT_SOURCE_DIR}/spdlog)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(spdlog PRIVATE --no-warnings)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(spdlog PRIVATE -w)
elseif(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  target_compile_options(spdlog PRIVATE /W0)
endif()

# Libusb
init_git_submodule(${CMAKE_CURRENT_SOURCE_DIR}/libusb-cmake)
