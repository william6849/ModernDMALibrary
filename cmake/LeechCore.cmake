include(ExternalProject)

set_directory_properties(PROPERTIES EP_BASE ${CMAKE_BINARY_DIR}/third_party)

execute_process(
  COMMAND curl -s https://api.github.com/repos/ufrisk/LeechCore/releases/latest
  OUTPUT_VARIABLE RELEASE_JSON)

if(WIN32)
  set(OS_TYPE win_x64)
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(OS_TYPE aarch64)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
    message(WARNING "x32 platform is not support currently.")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(OS_TYPE linux_x64)
  endif()
endif()

string(REGEX MATCH "\"browser_download_url\": \"([^\"]*LeechCore_files[^\"]*${OS_TYPE}[^\"]*)\"" _
  "${RELEASE_JSON}")
set(RELEASE_URL ${CMAKE_MATCH_1})
string(REGEX MATCH "(v[0-9]+\.[0-9]+\.[0-9]+)" RELEASE_VERSION "${RELEASE_URL}")
string(REGEX MATCH "-([0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9])." _
  "${RELEASE_URL}")
set(RELEASE_DATE ${CMAKE_MATCH_1})
message("Fetch Latest LeechCore Release: " ${RELEASE_VERSION} " LastUpdate: "
  ${RELEASE_DATE})
message(VERBOSE "Target Release: " ${RELEASE_URL})

ExternalProject_Add(
  LeechCore
  URL ${RELEASE_URL}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

ExternalProject_Get_Property(LeechCore SOURCE_DIR)
set(LEECHCORE_RESOURCE_DIR ${SOURCE_DIR})
set_target_properties(
  LeechCore
  PROPERTIES BUILD_RPATH "."
  INSTALL_RPATH "."
  INSTALL_RPATH_USE_LINK_PATH TRUE)

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
