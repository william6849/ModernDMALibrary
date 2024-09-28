file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.h")

set(TARGET_LIBRARY_NAME MDMAL)

if(BUILD_SHARED_LIBS)
  add_library(${TARGET_LIBRARY_NAME} SHARED ${SOURCE_FILES})
  target_compile_definitions(${TARGET_LIBRARY_NAME}
                             PUBLIC MDMAL_LIB_SHARED_BUILD)
  target_compile_definitions(${TARGET_LIBRARY_NAME} PRIVATE MDMAL_LIB_EXPORTS)
else()
  add_library(${TARGET_LIBRARY_NAME} STATIC ${SOURCE_FILES})
endif()

if(WIN32)
  target_compile_definitions(${TARGET_LIBRARY_NAME} PRIVATE WIN32_LEAN_AND_MEAN
                                                            NTSTATUS=LONG)
else()
  target_compile_definitions(${TARGET_LIBRARY_NAME} PRIVATE LINUX)
endif()

target_include_directories(
  ${TARGET_LIBRARY_NAME}
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
          ${CMAKE_BINARY_DIR}/third_party/Source/LeechCore
          ${CMAKE_BINARY_DIR}/third_party/Source/MemProcFS)

target_include_directories(
  ${TARGET_LIBRARY_NAME} SYSTEM
  PRIVATE ${CMAKE_SOURCE_DIR}/submodules/libusb-cmake/libusb/libusb
          ${CMAKE_SOURCE_DIR}/submodules/spdlog/include
          ${CMAKE_SOURCE_DIR}/submodules/googletest/googletest/include)
add_library(third_party_libs INTERFACE IMPORTED)
set_property(TARGET third_party_libs PROPERTY INTERFACE_LINK_LIBRARIES
                                              leechcorelib memprocfslib)
target_link_libraries(${TARGET_LIBRARY_NAME} PRIVATE third_party_libs)

add_dependencies(${TARGET_LIBRARY_NAME} usb-1.0)
add_dependencies(${TARGET_LIBRARY_NAME} spdlog)

if(RUN_CLANG_TIDY_ON_TARGET)
  set_target_properties(
    ${TARGET_LIBRARY_NAME} PROPERTIES CXX_CLANG_TIDY "clang-tidy"
                                      EXPORT_COMPILE_COMMANDS "ON")
endif()

target_code_coverage(${TARGET_LIBRARY_NAME})

# install settings
install(
  TARGETS ${TARGET_LIBRARY_NAME}
  EXPORT ${TARGET_LIBRARY_NAME}Targets
  RUNTIME DESTINATION "bin"
  LIBRARY DESTINATION "lib"
  ARCHIVE DESTINATION "lib")

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
  DESTINATION "include"
  FILES_MATCHING
  PATTERN "*.h")

install(
  EXPORT ${TARGET_LIBRARY_NAME}Targets
  FILE ${TARGET_LIBRARY_NAME}Targets.cmake
  NAMESPACE ${TARGET_LIBRARY_NAME}::
  DESTINATION lib/cmake/${TARGET_LIBRARY_NAME})

include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_LIBRARY_NAME}Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}Config.cmake"
  INSTALL_DESTINATION "lib/cmake/${TARGET_LIBRARY_NAME}")
# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}ConfigVersion.cmake"
  VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
  COMPATIBILITY AnyNewerVersion)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}ConfigVersion.cmake
  DESTINATION lib/cmake/${TARGET_LIBRARY_NAME})

export(
  EXPORT ${TARGET_LIBRARY_NAME}Targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}Targets.cmake
  NAMESPACE ${TARGET_LIBRARY_NAME}::)
