# Install script for directory: /home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SEAL-3.5/seal" TYPE FILE FILES
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/batchencoder.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/biguint.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/ciphertext.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/ckks.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/modulus.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/context.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/decryptor.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/intencoder.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/encryptionparams.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/encryptor.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/evaluator.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/galoiskeys.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/intarray.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/keygenerator.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/kswitchkeys.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/memorymanager.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/plaintext.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/publickey.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/randomgen.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/randomtostd.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/relinkeys.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/seal.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/secretkey.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/serializable.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/serialization.h"
    "/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/valcheck.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/wonderwoman/desktop/SEAL_N/SEAL-master/native/src/seal/util/cmake_install.cmake")

endif()

