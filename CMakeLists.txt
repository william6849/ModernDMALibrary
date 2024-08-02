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

# Third party library
include(cmake/LeechCore.cmake)
include(cmake/MemProcFS.cmake)

add_subdirectory(submodules)
add_subdirectory(MordenDMALibrary)
add_subdirectory(examples)
