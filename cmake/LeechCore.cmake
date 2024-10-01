include(ExternalProject)

set_directory_properties(PROPERTIES EP_BASE ${CMAKE_BINARY_DIR}/third_party)

if(WIN32)
  set(RELEASE_URL
        https://github.com/ufrisk/LeechCore/releases/download/v2.19/LeechCore_files_and_binaries_v2.19.1-win_x64-20240930.zip
  )
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(RELEASE_URL
        https://github.com/ufrisk/LeechCore/releases/download/v2.19/LeechCore_files_and_binaries_v2.19.1-linux_aarch64-20240930.tar.gz
    )
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
    message(WARNING "x32 platform is not support currently.")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(RELEASE_URL
        https://github.com/ufrisk/LeechCore/releases/download/v2.19/LeechCore_files_and_binaries_v2.19.1-linux_x64-20240930.tar.gz
    )
  endif()
endif()

message("Fetch LeechCore Release")
message(VERBOSE "Target Release: " ${RELEASE_URL})

ExternalProject_Add(
  LeechCore
  URL ${RELEASE_URL}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

ExternalProject_Get_Property(LeechCore SOURCE_DIR)
set(LEECHCORE_RESOURCE_DIR ${SOURCE_DIR})
set_target_properties(LeechCore PROPERTIES
    BUILD_RPATH "."
    INSTALL_RPATH "."
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

add_library(leechcorelib SHARED IMPORTED GLOBAL)
add_dependencies(leechcorelib LeechCore)
set_property(TARGET leechcorelib
             PROPERTY IMPORTED_IMPLIB ${LEECHCORE_RESOURCE_DIR}/leechcore.lib)

set_property(
  TARGET leechcorelib
  PROPERTY IMPORTED_LOCATION
           ${LEECHCORE_RESOURCE_DIR}/leechcore${CMAKE_SHARED_LIBRARY_SUFFIX})

if(WIN32)
  add_custom_command(
    TARGET LeechCore
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_BINARY_DIR}/third_party/Source/LeechCore/leechcore.dll
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/leechcore.dll)
endif()
