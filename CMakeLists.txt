cmake_minimum_required(VERSION 3.26)
project(MDL VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build flags for CXX
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O1")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
    "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS_DEBUG}")

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  add_compile_defin3itions(WIN32_LEAN_AND_MEAN)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_compile_definitions(LINUX)
  include(ExternalProject)
  ExternalProject_Add(
    leechcore
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/LeechCore/leechcore
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "make"
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE ON)
endif()

# Check dependencies
find_library(libusb_path NAMES usb-1.0 ${CMAKE_CURRENT_SOURCE_DIR/lib})
if(${libusb_path} STREQUAL "libusb_path-NOTFOUND")
  message(FATAL_ERROR "Please provide or install libusb-1.0")
else()
  message("Found libusb at: ${libusb_path}")
endif()

# Import libraries
link_directories(${CMAKE_SOURCE_DIR}/LeechCore/files)

# Google test module
add_subdirectory(googletest)
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)

add_subdirectory(spdlog)

# Build the library
add_library(MDMAL SHARED main.cc)
target_include_directories(
  MDMAL
  PUBLIC ${CMAKE_SOURCE_DIR}/include googletest/googletest/include
         LeechCore/includes spdlog/include ${CMAKE_INSTALL_PREFIX}/include)
target_link_directories(MDMAL PRIVATE leechcore)
