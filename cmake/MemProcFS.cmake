include(ExternalProject)

set_directory_properties(PROPERTIES EP_BASE ${CMAKE_BINARY_DIR}/third_party)

if(WIN32)
  set(RELEASE_URL
      https://github.com/ufrisk/MemProcFS/releases/download/v5.11/MemProcFS_files_and_binaries_v5.11.6-win_x64-20240917.zip
  )
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(RELEASE_URL
        https://github.com/ufrisk/MemProcFS/releases/download/v5.11/MemProcFS_files_and_binaries_v5.11.6-linux_aarch64-20240917.tar.gz
    )
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
    message(WARNING "x32 platform is not support currently.")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(RELEASE_URL
        https://github.com/ufrisk/MemProcFS/releases/download/v5.11/MemProcFS_files_and_binaries_v5.11.6-linux_x64-20240917.tar.gz
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
set_target_properties(MemProcFS PROPERTIES
    BUILD_RPATH "."
    INSTALL_RPATH "."
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

add_library(memprocfslib SHARED IMPORTED GLOBAL)
add_dependencies(memprocfslib MemProcFS)
set_property(TARGET memprocfslib PROPERTY IMPORTED_IMPLIB
                                          ${MEMPROCFS_RESOURCE_DIR}/vmm.lib)
set_property(
  TARGET memprocfslib
  PROPERTY IMPORTED_LOCATION
           ${MEMPROCFS_RESOURCE_DIR}/vmm${CMAKE_SHARED_LIBRARY_SUFFIX})
if(WIN32)
  add_custom_command(
    TARGET MemProcFS
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_BINARY_DIR}/third_party/Source/MemProcFs/vmm.dll
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vmm.dll)
endif()
