include(ExternalProject)

set_directory_properties(PROPERTIES EP_BASE ${CMAKE_BINARY_DIR}/third_party)

if(WIN32)
  set(RELEASE_URL
      https://github.com/ufrisk/MemProcFS/releases/download/v5.10/MemProcFS_files_and_binaries_v5.10.1-win_x64-20240721.zip
  )
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(RELEASE_URL
        https://github.com/ufrisk/MemProcFS/releases/download/v5.10/MemProcFS_files_and_binaries_v5.10.1-linux_aarch64-20240721.tar.gz
    )
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
    message(WARNING "x32 platform is not support currently.")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(RELEASE_URL
        https://github.com/ufrisk/MemProcFS/releases/download/v5.10/MemProcFS_files_and_binaries_v5.10.1-linux_x64-20240721.tar.gz
    )
  endif()
endif()

message("Fetch MemProcFS Release")
message(VERBOSE "Target Release: " ${RELEASE_URL})

ExternalProject_Add(
  MemProcFS
  URL ${RELEASE_URL}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

ExternalProject_Get_Property(MemProcFS SOURCE_DIR)
set(MEMPROCFS_RESOURCE_DIR ${SOURCE_DIR})

file(GLOB MEMPROCFS_HEADER_LIST ${SOURCE_DIR}/*.h)
if(WIN32)
  file(GLOB MEMPROCFS_LIB_LIST ${SOURCE_DIR}/*.lib)
  file(GLOB MEMPROCFS_SHARED_LIST ${SOURCE_DIR}/*.dll)
else()
  file(GLOB MEMPROCFS_SHARED_LIST ${SOURCE_DIR}/*.so)
endif()

add_library(memprocfslib SHARED IMPORTED)
add_dependencies(memprocfslib MemProcFS)
if(WIN32)
  set_property(TARGET memprocfslib PROPERTY IMPORTED_IMPLIB
                                            ${MEMPROCFS_RESOURCE_DIR}/vmm.lib)
endif()
set_property(TARGET memprocfslib PROPERTY IMPORTED_LOCATION
                                          ${MEMPROCFS_SHARED_LIST})
