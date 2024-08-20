add_executable(MDMAB main.cc)

if(WIN32)
  target_compile_definitions(MDMAB PRIVATE WIN32_LEAN_AND_MEAN NTSTATUS=LONG)
else()
  target_compile_definitions(MDMAB PRIVATE LINUX)
endif()

target_include_directories(
  MDMAB SYSTEM PRIVATE ${CMAKE_BINARY_DIR}/third_party/Source/LeechCore
                       ${CMAKE_BINARY_DIR}/third_party/Source/MemProcFS)

target_link_libraries(MDMAB MDMAL)
