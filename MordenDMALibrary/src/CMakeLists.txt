link_directories(${CMAKE_BINARY_DIR}/files)

include_directories(
${CMAKE_SOURCE_DIR}/googletest/googletest/include 
${CMAKE_SOURCE_DIR}/spdlog/include 
${CMAKE_SOURCE_DIR}/libusb-cmake/libusb/libusb
${CMAKE_SOURCE_DIR}/MemProcFS/includes
${CMAKE_SOURCE_DIR}/LeechCore/includes
${CMAKE_SOURCE_DIR}/MordenDMALibrary/include 
${CMAKE_INSTALL_PREFIX}/include)

add_library(MDMAL STATIC device.cc memory.cc)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(MDMAL BEFORE PUBLIC ${CMAKE_BINARY_DIR}/includes)
target_link_libraries(MDMAL PRIVATE libusb-1.0)

add_dependencies(MDMAL leechcore)
add_dependencies(MDMAL vmm)
add_dependencies(MDMAL memprocfs)
add_dependencies(MDMAL spdlog)