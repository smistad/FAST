
if(NOT "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO-stamp/OpenVINO-gitinfo.txt" IS_NEWER_THAN "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO-stamp/OpenVINO-gitclone-lastrun.txt")
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: '/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO-stamp/OpenVINO-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"  clone --no-checkout "https://github.com/openvinotoolkit/openvino.git" "OpenVINO"
    WORKING_DIRECTORY "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/openvinotoolkit/openvino.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"  checkout 2020.3.0 --
  WORKING_DIRECTORY "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: '2020.3.0'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git"  submodule update --recursive --init 
    WORKING_DIRECTORY "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO"
    RESULT_VARIABLE error_code
    )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO-stamp/OpenVINO-gitinfo.txt"
    "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO-stamp/OpenVINO-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO-stamp/OpenVINO-gitclone-lastrun.txt'")
endif()

