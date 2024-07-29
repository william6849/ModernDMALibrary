cmake_minimum_required(VERSION 3.26)
project(MDL VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build flags for CXX
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O1")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
#    "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS_DEBUG}")

if(WIN32)
  include_external_msproject(leechcore ${CMAKE_SOURCE_DIR}/LeechCore/leechcore/leechcore.vcxproj)
  include_external_msproject(vmm ${CMAKE_SOURCE_DIR}/MemProcFS/vmm/vmm.vcxproj leechcore)
  include_external_msproject(memprocfs ${CMAKE_SOURCE_DIR}/MemProcFS/memprocfs/memprocfs.vcxproj vmm)
else()
  add_compile_definitions(LINUX)
  include(ExternalProject)
  ExternalProject_Add(
    leechcore
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/LeechCore/leechcore
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "make"
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE ON)
    link_directories(${CMAKE_SOURCE_DIR}/LeechCore/files)

    ExternalProject_Add(
      vmm
      SOURCE_DIR ${CMAKE_SOURCE_DIR}/MemProcFS/vmm
      CONFIGURE_COMMAND ""
      BUILD_COMMAND "make"
      INSTALL_COMMAND ""
      BUILD_IN_SOURCE ON)
      link_directories(${CMAKE_SOURCE_DIR}/MemProcFS/files)
      ExternalProject_Add(
         memprocfs
         SOURCE_DIR ${CMAKE_SOURCE_DIR}/MemProcFS/memprocfs
         CONFIGURE_COMMAND ""
         BUILD_COMMAND "make"
         INSTALL_COMMAND ""
         BUILD_IN_SOURCE ON)
         link_directories(${CMAKE_SOURCE_DIR}/MemProcFS/files)
endif()

# Google test module
add_subdirectory(googletest)
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)

# Spdlog
add_subdirectory(spdlog)
if (CMAKE_COMPILER_IS_GNUCC)
target_compile_options(spdlog PRIVATE -w)
endif()
if (MSVC)
target_compile_options(spdlog PRIVATE /W0)
endif()

# Libusb
add_subdirectory(libusb-cmake)
link_directories(${CMAKE_BINARY_DIR}/libusb-cmake)

add_dependencies(vmm leechcore)

add_subdirectory(examples)
add_subdirectory(MordenDMALibrary)