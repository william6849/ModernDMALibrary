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

ExternalProject_Get_Property(LeechCore SOURCE_DIR)
set(LEECHCORE_RESOURCE_DIR ${SOURCE_DIR})

file(GLOB LEECHCORE_HEADER_LIST ${SOURCE_DIR}/*.h)
if(WIN32)
  file(GLOB LEECHCORE_LIB_LIST ${SOURCE_DIR}/*.lib)
  file(GLOB LEECHCORE_SHARED_LIST ${SOURCE_DIR}/*.dll)
else()
  file(GLOB LEECHCORE_SHARED_LIST ${SOURCE_DIR}/*.so)
endif()

add_library(leechcorelib SHARED IMPORTED)
add_dependencies(leechcorelib LeechCore)
if(WIN32)
  set_property(TARGET leechcorelib
               PROPERTY IMPORTED_IMPLIB ${LEECHCORE_RESOURCE_DIR}/leechcore.lib)
endif()
set_property(TARGET leechcorelib PROPERTY IMPORTED_LOCATION
                                          ${LEECHCORE_SHARED_LIST})
