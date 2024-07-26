cmake_minimum_required(VERSION 3.14)
project(MDL VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(googletest)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(spdlog)

add_custom_target(
   extern_lib
   COMMAND make
   WORKING_DIRECTORY LeechCore/leechcore
)