add_executable(MDMAB main.cc)
target_link_libraries(MDMAB MDMAL)

if(WIN32)
  add_custom_command(
    TARGET MDMAB
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_BINARY_DIR}/third_party/Source/LeechCore/leechcore.dll
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/leechcore.dll)

  add_custom_command(
    TARGET MDMAB
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_BINARY_DIR}/third_party/Source/MemProcFs/vmm.dll
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/vmm.dll)
else()

endif()
