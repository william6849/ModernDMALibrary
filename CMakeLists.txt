cmake_minimum_required(VERSION 3.26)
project(MordenDMALibrary VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(BUILD_SHARED_LIBS
       "Specifies the type of libraries (SHARED or STATIC) to build" OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install/MordenDMALibrary")
endif()

if(WIN32)

else()
  add_compile_definitions(LINUX)
endif()

# Third party library
include(cmake/LeechCore.cmake)
include(cmake/MemProcFS.cmake)

# Google test module
add_subdirectory(googletest)
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)

# Spdlog
add_subdirectory(spdlog)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(spdlog PRIVATE --no-warnings)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(spdlog PRIVATE -w)
elseif(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  target_compile_options(spdlog PRIVATE /W0)
endif()

# Libusb
add_subdirectory(libusb-cmake)

add_subdirectory(MordenDMALibrary)
add_subdirectory(examples)
