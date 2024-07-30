include(ExternalProject)

set(ROOT_DIR ${CMAKE_BINARY_DIR}/third_party/MemProcFS)

if(WIN32)
  set(URL
      https://github.com/ufrisk/MemProcFS/releases/download/v5.10/MemProcFS_files_and_binaries_v5.10.1-win_x64-20240721.zip
  )
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(URL
        https://github.com/ufrisk/MemProcFS/releases/download/v5.10/MemProcFS_files_and_binaries_v5.10.1-linux_aarch64-20240721.tar.gz
    )
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
    message(WARNING "x32 platform is not support currently.")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(URL
        https://github.com/ufrisk/MemProcFS/releases/download/v5.10/MemProcFS_files_and_binaries_v5.10.1-linux_x64-20240721.tar.gz
    )
  endif()
endif()

message("Setup MemProcFS")
message("Platform: " ${CMAKE_SYSTEM_PROCESSOR})
message("Get: " ${URL})

ExternalProject_Add(
  MemProcFS
  URL ${URL}
  PREFIX ${CMAKE_BINARY_DIR}
  SOURCE_DIR ${ROOT_DIR}
  BINARY_DIR ${ROOT_DIR}
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND "")
