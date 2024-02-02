# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

project(Automotive-Bus_Codec)

include(FetchContent)

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


# External Project - Automotive Bus Schema
# -------------------------------------------
FetchContent_Declare(automotive-bus-schema
    URL https://github.com/boschglobal/automotive-bus-schema/releases/download/v1.0.3/automotive-bus-schema.tar.gz
)
FetchContent_MakeAvailable(automotive-bus-schema)
set(SCHEMAS_SOURCE_DIR ${automotive-bus-schema_SOURCE_DIR}/flatbuffers/c)
set(FLATCC_SOURCE_DIR ${SCHEMAS_SOURCE_DIR}/automotive_bus_schema/flatcc/src)
set(FLATCC_INCLUDE_DIR ${SCHEMAS_SOURCE_DIR}/automotive_bus_schema/flatcc/include)
set(FLATCC_SOURCE_FILES
    ${FLATCC_SOURCE_DIR}/builder.c
    ${FLATCC_SOURCE_DIR}/emitter.c
    ${FLATCC_SOURCE_DIR}/refmap.c
)


# Targets
# =======
add_library(automotive-bus-codec
    STATIC
        codec.c
        frame_can_fbs.c
        ${FLATCC_SOURCE_FILES}
)
target_include_directories(automotive-bus-codec
    PRIVATE
        ${FLATCC_INCLUDE_DIR}
        ${SCHEMAS_SOURCE_DIR}
        ..
        ../../../..
)
