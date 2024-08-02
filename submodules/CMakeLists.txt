# Google test module
add_subdirectory(googletest)
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)

# Spdlog
add_subdirectory(spdlog)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(spdlog PRIVATE --no-warnings)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(spdlog PRIVATE -w)
elseif(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  target_compile_options(spdlog PRIVATE /W0)
endif()

# Libusb
add_subdirectory(libusb-cmake)
