# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.
#message(${SEALExamples_SOURCE_DIR})

cmake_minimum_required(VERSION 3.12)

project(SEAL VERSION 3.5.9 LANGUAGES CXX C)

# Executable will be in ../bin
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SEALExamples_SOURCE_DIR}/bin)

add_executable(test1 
Sigmoid.cpp 
Sign_algo1.cpp 
Sign_algo2.cpp 
Deep_neural_nw.cpp 
Discrete_neural_nw.cpp 
Performance_test.cpp )


# Import Microsoft SEAL
find_package(SEAL 3.5.9 REQUIRED
    # Providing a path so this can be built without installing Microsoft SEAL
    PATHS ${SEALExamples_SOURCE_DIR}/../src/seal
)
# message(${SEALExamples_SOURCE_DIR})


# Link Microsoft SEAL

#target_link_libraries(deep_nw SEAL::seal)
target_link_libraries(test1 SEAL::seal)


