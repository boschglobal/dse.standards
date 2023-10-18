# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

project(Example)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3")
list(APPEND C_CXX_WARNING_FLAGS
    -Wall
    -W
    -Wwrite-strings
    -Wno-missing-field-initializers
    -Wno-misleading-indentation
    -Winline
)
add_compile_options(${C_CXX_WARNING_FLAGS})


# Targets
# =======
add_library(codec
    SHARED
        codec.c
)
target_include_directories(codec
    PRIVATE
        ../../..
)

add_executable(example_dynamic
    dynamic.c
    example.c
    stream.c
    ../../../dse/ncodec/codec.c
)
target_include_directories(example_dynamic
    PRIVATE
        ../../..
)
target_link_libraries(example_dynamic
    PRIVATE
        dl
)

add_executable(example_static
    codec.c
    example.c
    static.c
    stream.c
    ../../../dse/ncodec/codec.c
)
target_include_directories(example_static
    PRIVATE
        ../../..
)
