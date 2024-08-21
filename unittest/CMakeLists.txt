enable_testing()

add_executable(dmatest dmatest.cc)
target_link_libraries(dmatest GTest::gtest_main MDMAL)
if(CODE_COVERAGE)
  target_link_options(dmatest PRIVATE "--coverage")
endif()

if(WIN32)
  target_compile_definitions(dmatest PRIVATE WIN32_LEAN_AND_MEAN NTSTATUS=LONG)
else()
  target_compile_definitions(dmatest PRIVATE LINUX)
endif()

target_include_directories(
  dmatest SYSTEM
  PRIVATE ${CMAKE_SOURCE_DIR}/submodules/spdlog/include
          ${CMAKE_BINARY_DIR}/third_party/Source/LeechCore
          ${CMAKE_BINARY_DIR}/third_party/Source/MemProcFS)

include(GoogleTest)
gtest_discover_tests(dmatest)
