file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.h")

set(ROOT_DIR ${CMAKE_SOURCE_DIR}/MordenDMALibrary)
set(TARGET_LIBRARY_NAME MDMAL)

if(BUILD_SHARED_LIBS)
  add_library(${TARGET_LIBRARY_NAME} SHARED ${SOURCE_FILES})
  target_compile_definitions(${TARGET_LIBRARY_NAME}
                             PUBLIC -DMDMAL_LIB_SHARED_BUILD)
  target_compile_definitions(${TARGET_LIBRARY_NAME} PRIVATE -DMDMAL_LIB_EXPORTS)
else()
  add_library(${TARGET_LIBRARY_NAME} STATIC ${SOURCE_FILES})
endif()

target_include_directories(
  ${TARGET_LIBRARY_NAME}
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(
  ${TARGET_LIBRARY_NAME} SYSTEM
  PRIVATE ${CMAKE_SOURCE_DIR}/libusb-cmake/libusb/libusb
          ${CMAKE_SOURCE_DIR}/spdlog/include
          ${CMAKE_SOURCE_DIR}/googletest/googletest/include
          ${CMAKE_BINARY_DIR}/third_party/LeechCore
          ${CMAKE_BINARY_DIR}/third_party/MemProcFS)

add_library(leechcore SHARED IMPORTED)
if(WIN32)
  set_property(
    TARGET leechcore
    PROPERTY IMPORTED_IMPLIB
             ${CMAKE_BINARY_DIR}/third_party/LeechCore/leechcore.lib)
else()
  set_property(
    TARGET leechcore
    PROPERTY IMPORTED_LOCATION
             ${CMAKE_BINARY_DIR}/third_party/LeechCore/leechcore.so)
endif()

add_library(memprocfs SHARED IMPORTED)
if(WIN32)
  set_property(
    TARGET memprocfs PROPERTY IMPORTED_IMPLIB
                              ${CMAKE_BINARY_DIR}/third_party/MemProcFS/vmm.lib)
else()
  set_property(
    TARGET memprocfs PROPERTY IMPORTED_LOCATION
                              ${CMAKE_BINARY_DIR}/third_party/MemProcFS/vmm.so)
endif()

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

target_link_libraries(${TARGET_LIBRARY_NAME} PRIVATE leechcore)
target_link_libraries(${TARGET_LIBRARY_NAME} PRIVATE memprocfs)

add_dependencies(${TARGET_LIBRARY_NAME} usb-1.0)
add_dependencies(${TARGET_LIBRARY_NAME} spdlog)

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
