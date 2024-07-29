include(ExternalProject)

set(ROOT ${CMAKE_BINARY_DIR}/third_party/LeechCore)
set(LIB_DIR ${ROOT}/lib)
set(INCLUDE_DIR ${ROOT}/include)

set(BUILD_COMMAND
    "cp ${ROOT}/src/LeechCore/*.h ${CMAKE_BINARY_DIR}/include && cp ${ROOT}/src/LeechCore/*.${CMAKE_SHARED_LIBRARY_PREFIX} ${CMAKE_BINARY_DIR}/include"
)

if(WIN32)
  set(URL
      https://github.com/ufrisk/LeechCore/releases/download/v2.18/LeechCore_files_and_binaries_v2.18.7-win_x64-20240721.zip
  )
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(URL
        https://github.com/ufrisk/LeechCore/releases/download/v2.18/LeechCore_files_and_binaries_v2.18.7-linux_aarch64-20240721.tar.gz
    )
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
    message(WARNING "x32 platform is not support currently.")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(URL
        https://github.com/ufrisk/LeechCore/releases/download/v2.18/LeechCore_files_and_binaries_v2.18.7-linux_x64-20240721.tar.gz
    )
  endif()
endif()

message("Setup LeechCore")
message("Platform: " ${CMAKE_SYSTEM_PROCESSOR})
message("Get: " ${URL})

ExternalProject_Add(
  LeechCore
  URL ${URL}
  PREFIX ${ROOT}
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND "")
