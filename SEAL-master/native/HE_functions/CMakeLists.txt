# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.
cmake_minimum_required(VERSION 3.12)

project(SEAL VERSION 3.5.9 LANGUAGES CXX C)

# Executable will be in ../bin
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SEALExamples_SOURCE_DIR}/../bin)

add_executable(obj rotate.cpp examples.h)
# Import Microsoft SEAL
find_package(SEAL 3.5.9 REQUIRED
    # Providing a path so this can be built without installing Microsoft SEAL
    PATHS ${SEALExamples_SOURCE_DIR}/../src/seal
)


# Link Microsoft SEAL


target_link_libraries(obj SEAL::seal)
