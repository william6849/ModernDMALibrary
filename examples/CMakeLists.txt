# Add test executable
include_directories(${CMAKE_SOURCE_DIR}/MordenDMALibrary/include)
link_directories(${CMAKE_BINARY_DIR}/MordenDMALibrary/src)
add_executable(MDMAB main.cc)
target_link_libraries(MDMAB memprocfs)
target_link_libraries(MDMAB MDMAL)
