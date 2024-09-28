enable_testing()

add_executable(unittest dmatest.cc)
target_link_libraries(unittest GTest::gtest_main MDMAL)
target_code_coverage(unittest)

if(WIN32)
  target_compile_definitions(unittest PRIVATE WIN32_LEAN_AND_MEAN NTSTATUS=LONG)
else()
  target_compile_definitions(unittest PRIVATE LINUX)
endif()

target_include_directories(
  unittest SYSTEM
  PRIVATE ${CMAKE_SOURCE_DIR}/submodules/spdlog/include
          ${CMAKE_BINARY_DIR}/third_party/Source/LeechCore
          ${CMAKE_BINARY_DIR}/third_party/Source/MemProcFS)

include(GoogleTest)
gtest_discover_tests(unittest)
