add_executable(MDMAB main.cc)
target_include_directories(MDMAB
                           PRIVATE ${CMAKE_SOURCE_DIR}/MordenDMALibrary/include)
target_link_libraries(MDMAB MDMAL)

if(WIN32)
  add_custom_command(
    TARGET MDMAB
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy
      ${CMAKE_BINARY_DIR}/third_party/LeechCore/leechcore.dll
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/leechcore.dll)

  add_custom_command(
    TARGET MDMAB
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy
      ${CMAKE_BINARY_DIR}/third_party/MemProcFs/vmm.dll
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/vmm.dll)
else()

endif()
