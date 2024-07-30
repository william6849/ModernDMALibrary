# Add test executable
add_executable(MDMAB main.cc)
target_include_directories(MDMAB
                           PRIVATE ${CMAKE_SOURCE_DIR}/MordenDMALibrary/include)
target_link_libraries(MDMAB MDMAL)
