# Google test module
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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
if(LINUX)
  find_file(NAMES libudev.h PATHS /usr/include REQUIRED)
endif()
init_git_submodule(${CMAKE_CURRENT_SOURCE_DIR}/libusb-cmake)
