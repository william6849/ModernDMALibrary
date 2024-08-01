include(ExternalProject)

set_directory_properties(PROPERTIES EP_BASE ${CMAKE_BINARY_DIR}/third_party)

if(WIN32)
  set(RELEASE_URL
      https://github.com/ufrisk/LeechCore/releases/download/v2.18/LeechCore_files_and_binaries_v2.18.7-win_x64-20240721.zip
  )
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(RELEASE_URL
        https://github.com/ufrisk/LeechCore/releases/download/v2.18/LeechCore_files_and_binaries_v2.18.7-linux_aarch64-20240721.tar.gz
    )
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
    message(WARNING "x32 platform is not support currently.")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(RELEASE_URL
        https://github.com/ufrisk/LeechCore/releases/download/v2.18/LeechCore_files_and_binaries_v2.18.7-linux_x64-20240721.tar.gz
    )
  endif()
endif()

message("Fetch LeechCore Release")
message(VERBOSE "Target Release: " ${RELEASE_URL})

ExternalProject_Add(
  LeechCore
  URL ${RELEASE_URL}
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND "")
