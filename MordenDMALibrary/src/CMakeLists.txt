set(ROOT_DIR ${CMAKE_SOURCE_DIR}/MordenDMALibrary)

include_directories(
  ${CMAKE_SOURCE_DIR}/googletest/googletest/include
  ${CMAKE_SOURCE_DIR}/spdlog/include
  ${CMAKE_SOURCE_DIR}/libusb-cmake/libusb/libusb
  ${CMAKE_SOURCE_DIR}/MordenDMALibrary/include
  ${CMAKE_BINARY_DIR}/third_party/MemProcFS/src/MemProcFS
  ${CMAKE_INSTALL_PREFIX}/include)

add_library(memprocfs SHARED IMPORTED GLOBAL)
set_property(
  TARGET memprocfs
  PROPERTY IMPORTED_LOCATION
           ${CMAKE_BINARY_DIR}/third_party/MemProcFS/src/MemProcFS/vmm.so)

file(GLOB SOURCES "${ROOT_DIR}/src/*.cc")
file(GLOB HEADERS "${ROOT_DIR}/include/*.h")
add_library(MDMAL ${SOURCES} ${HEADERS})

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

target_link_libraries(MDMAL memprocfs)
target_link_libraries(MDMAL usb-1.0)

add_dependencies(MDMAL LeechCore)
add_dependencies(MDMAL MemProcFS)
add_dependencies(MDMAL spdlog)
