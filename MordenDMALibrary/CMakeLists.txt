file(GLOB_RECURSE  SOURCE_FILES "*.cc")
file(GLOB_RECURSE  HEADER_FILES "*.h")

set(ROOT_DIR ${CMAKE_SOURCE_DIR}/MordenDMALibrary)
set(TARGET_LIBRARY_NAME MDMAL)
set(MEMPROCFS_LIB_NAME memprocfs)

if (BUILD_SHARED_LIBS)
    add_library(${TARGET_LIBRARY_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})
    target_compile_definitions(${TARGET_LIBRARY_NAME} PUBLIC -DMDMAL_LIB_SHARED_BUILD)
    target_compile_definitions(${TARGET_LIBRARY_NAME} PRIVATE -DMDMAL_LIB_EXPORTS)
else()
    add_library(${TARGET_LIBRARY_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})
endif()

target_include_directories(${TARGET_LIBRARY_NAME}
PUBLIC
$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
$<INSTALL_INTERFACE:include>
)

add_library(${MEMPROCFS_LIB_NAME} SHARED IMPORTED GLOBAL)
if(WIN32)
set_property(
  TARGET ${MEMPROCFS_LIB_NAME}
  PROPERTY IMPORTED_LOCATION
           ${CMAKE_BINARY_DIR}/third_party/MemProcFS/src/MemProcFS/vmm.dll)
           set_property(
  TARGET ${MEMPROCFS_LIB_NAME}
  PROPERTY IMPORTED_IMPLIB
           ${CMAKE_BINARY_DIR}/third_party/LeechCore/src/LeechCore/leechcore.dll)
else()
endif()

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_dependencies(${TARGET_LIBRARY_NAME} LeechCore)
add_dependencies(${TARGET_LIBRARY_NAME} ${MEMPROCFS_LIB_NAME})
add_dependencies(${TARGET_LIBRARY_NAME} usb-1.0)
add_dependencies(${TARGET_LIBRARY_NAME} spdlog)

# install settings
install(TARGETS ${TARGET_LIBRARY_NAME}
EXPORT ${TARGET_LIBRARY_NAME}Targets
RUNTIME DESTINATION "bin"
LIBRARY DESTINATION "lib"
ARCHIVE DESTINATION "lib"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION "include"
        FILES_MATCHING PATTERN "*.h")

install(EXPORT ${TARGET_LIBRARY_NAME}Targets
    FILE ${TARGET_LIBRARY_NAME}Targets.cmake
    NAMESPACE ${TARGET_LIBRARY_NAME}::
    DESTINATION lib/cmake/${TARGET_LIBRARY_NAME}
)

include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_LIBRARY_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}Config.cmake"
    INSTALL_DESTINATION "lib/cmake/${TARGET_LIBRARY_NAME}"
)
# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}ConfigVersion.cmake"
    VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}ConfigVersion.cmake
    DESTINATION lib/cmake/${TARGET_LIBRARY_NAME}
)

export(EXPORT ${TARGET_LIBRARY_NAME}Targets FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_LIBRARY_NAME}Targets.cmake NAMESPACE ${TARGET_LIBRARY_NAME}::)