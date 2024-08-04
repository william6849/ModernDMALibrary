find_package(Git REQUIRED)

function(init_git_submodule module)

  if(NOT EXISTS ${module}/CMakeLists.txt)
    message(${module})
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${module}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/submodules COMMAND_ERROR_IS_FATAL
                        ANY)
  endif()
  add_subdirectory(${module})

endfunction(init_git_submodule)
