# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

project(bus-topology)

include(FetchContent)
include(ExternalProject)

# set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ggdb")
list(APPEND C_CXX_WARNING_FLAGS
    -Wall
    -W
    -Wwrite-strings
    -Wno-missing-field-initializers
    -Wno-misleading-indentation
)
add_compile_options(${C_CXX_WARNING_FLAGS})


# External Project - xml
# ----------------------
ExternalProject_Add(libxml2
    URL https://github.com/GNOME/libxml2/archive/refs/tags/v2.11.6.zip
    CMAKE_ARGS
        -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/libxml2
        -D CMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -D BUILD_SHARED_LIBS=OFF
        -D LIBXML2_WITH_HTML=OFF
        -D LIBXML2_WITH_HTTP=OFF
        -D LIBXML2_WITH_ICONV=OFF
        -D LIBXML2_WITH_LZMA=OFF
        -D LIBXML2_WITH_PYTHON=OFF
        -D LIBXML2_WITH_SCHEMATRON=OFF
        -D LIBXML2_WITH_TESTS=OFF
        -D LIBXML2_WITH_THREAD_ALLOC=OFF
        -D LIBXML2_WITH_THREADS=OFF
        -D LIBXML2_WITH_TREE=OFF
        -D LIBXML2_WITH_VALID=OFF
        -D LIBXML2_WITH_WRITER=OFF
        -D LIBXML2_WITH_ZLIB=OFF
    INSTALL_DIR "${CMAKE_BINARY_DIR}/libxml2"
    UPDATE_COMMAND ""
)
ExternalProject_Get_property(libxml2 SOURCE_DIR BINARY_DIR INSTALL_DIR)
set(XML_SOURCE_DIR "${SOURCE_DIR}")
set(XML_BINARY_DIR "${BINARY_DIR}")
set(XML_INSTALL_DIR "${INSTALL_DIR}")

add_library(xml STATIC IMPORTED GLOBAL)
add_dependencies(xml libxml2)
file(MAKE_DIRECTORY "${XML_SOURCE_DIR}/include")
file(MAKE_DIRECTORY "${XML_INSTALL_DIR}/include/libxml2")
set_target_properties(xml
    PROPERTIES
        IMPORTED_LOCATION "${XML_BINARY_DIR}/libxml2.a"
        INTERFACE_INCLUDE_DIRECTORIES "${XML_INSTALL_DIR}/include/libxml2"
)


# External Project - DSE C Lib
# ---------------------------
FetchContent_Declare(dse_clib
    URL https://github.com/boschglobal/dse.clib/archive/refs/tags/v1.0.17.tar.gz
)
FetchContent_MakeAvailable(dse_clib)
set(DSE_CLIB_SOURCE_DIR ${dse_clib_SOURCE_DIR}/dse)
set(DSE_CLIB_INCLUDE_DIR "${DSE_CLIB_SOURCE_DIR}/..")


# External Project - automotive_bus_schema
# ----------------------------------------
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


# External Project - DSE Network Codec
# ------------------------------------
FetchContent_Declare(dse_network_codec
    URL https://github.com/boschglobal/dse.standards/archive/refs/tags/v1.0.8.tar.gz
)
FetchContent_MakeAvailable(dse_network_codec)
set(DSE_NCODEC_SOURCE_DIR ${dse_network_codec_SOURCE_DIR}/dse/ncodec)
set(DSE_NCODEC_INCLUDE_DIR "${DSE_NCODEC_SOURCE_DIR}/../..")



# Targets
# =======

# Target - ncodec
# -------------------
add_library(ncodec OBJECT
    stream.c
    ncodec.c
    ${DSE_NCODEC_SOURCE_DIR}/codec.c
    ${DSE_NCODEC_SOURCE_DIR}/libs/automotive-bus/codec.c
    ${DSE_NCODEC_SOURCE_DIR}/libs/automotive-bus/frame_can_fbs.c
    ${FLATCC_SOURCE_FILES}
)
target_include_directories(ncodec
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
        ${DSE_NCODEC_INCLUDE_DIR}
        ${DSE_NCODEC_INCLUDE_DIR}/dse/ncodec/libs
        ${FLATCC_INCLUDE_DIR}
        ${SCHEMAS_SOURCE_DIR}
        ../../..
        ./
)


# Target - bus_topology
# -------------------
add_library(bus_topology OBJECT
    bus_topology.c
    parser.c
    ../../fmi-ls-binary-to-text/code/ascii85.c
    ${DSE_CLIB_SOURCE_DIR}/clib/collections/hashmap.c
)
target_include_directories(bus_topology
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
        ${DSE_NCODEC_INCLUDE_DIR}
        ./
)
target_link_libraries(bus_topology
    PUBLIC
        xml
)

add_subdirectory(tests)
add_subdirectory(example)
