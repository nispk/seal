# Install script for directory: /nisp/SEAL-master/native/src/seal

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
    "/nisp/SEAL-master/native/src/seal/batchencoder.h"
    "/nisp/SEAL-master/native/src/seal/biguint.h"
    "/nisp/SEAL-master/native/src/seal/ciphertext.h"
    "/nisp/SEAL-master/native/src/seal/ckks.h"
    "/nisp/SEAL-master/native/src/seal/modulus.h"
    "/nisp/SEAL-master/native/src/seal/context.h"
    "/nisp/SEAL-master/native/src/seal/decryptor.h"
    "/nisp/SEAL-master/native/src/seal/intencoder.h"
    "/nisp/SEAL-master/native/src/seal/encryptionparams.h"
    "/nisp/SEAL-master/native/src/seal/encryptor.h"
    "/nisp/SEAL-master/native/src/seal/evaluator.h"
    "/nisp/SEAL-master/native/src/seal/galoiskeys.h"
    "/nisp/SEAL-master/native/src/seal/intarray.h"
    "/nisp/SEAL-master/native/src/seal/keygenerator.h"
    "/nisp/SEAL-master/native/src/seal/kswitchkeys.h"
    "/nisp/SEAL-master/native/src/seal/memorymanager.h"
    "/nisp/SEAL-master/native/src/seal/plaintext.h"
    "/nisp/SEAL-master/native/src/seal/publickey.h"
    "/nisp/SEAL-master/native/src/seal/randomgen.h"
    "/nisp/SEAL-master/native/src/seal/randomtostd.h"
    "/nisp/SEAL-master/native/src/seal/relinkeys.h"
    "/nisp/SEAL-master/native/src/seal/seal.h"
    "/nisp/SEAL-master/native/src/seal/secretkey.h"
    "/nisp/SEAL-master/native/src/seal/serializable.h"
    "/nisp/SEAL-master/native/src/seal/serialization.h"
    "/nisp/SEAL-master/native/src/seal/valcheck.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/nisp/SEAL-master/native/src/seal/util/cmake_install.cmake")

endif()

