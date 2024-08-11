enable_testing()

add_executable(dmatest dmatest.cc)
target_link_libraries(dmatest GTest::gtest_main MDMAL)
if(CODE_COVERAGE)
  target_link_options(dmatest PRIVATE "--coverage")
endif()

include(GoogleTest)
gtest_discover_tests(dmatest)
