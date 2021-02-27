
if(NOT "/nisp/SEAL-master/thirdparty/zlib/stamp/EP_ZLIB-gitinfo.txt" IS_NEWER_THAN "/nisp/SEAL-master/thirdparty/zlib/stamp/EP_ZLIB-gitclone-lastrun.txt")
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: '/nisp/SEAL-master/thirdparty/zlib/stamp/EP_ZLIB-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E remove_directory "/nisp/SEAL-master/thirdparty/zlib/src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/nisp/SEAL-master/thirdparty/zlib/src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"  clone --config advice.detachedHead=false "https://github.com/madler/zlib.git" "src"
    WORKING_DIRECTORY "/nisp/SEAL-master/thirdparty/zlib"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/madler/zlib.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"  checkout v1.2.11 --
  WORKING_DIRECTORY "/nisp/SEAL-master/thirdparty/zlib/src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'v1.2.11'")
endif()

execute_process(
  COMMAND "/usr/bin/git"  submodule update --recursive --init 
  WORKING_DIRECTORY "/nisp/SEAL-master/thirdparty/zlib/src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/nisp/SEAL-master/thirdparty/zlib/src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "/nisp/SEAL-master/thirdparty/zlib/stamp/EP_ZLIB-gitinfo.txt"
    "/nisp/SEAL-master/thirdparty/zlib/stamp/EP_ZLIB-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/nisp/SEAL-master/thirdparty/zlib/stamp/EP_ZLIB-gitclone-lastrun.txt'")
endif()

