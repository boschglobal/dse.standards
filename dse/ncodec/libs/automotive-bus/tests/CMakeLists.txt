cmake_minimum_required(VERSION 3.21)

# set(CMAKE_VERBOSE_MAKEFILE ON)

project(test_can)

set(CMAKE_ENABLE_EXPORTS ON)

include(FetchContent)
include(GNUInstallDirs)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/_out)

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



# External Project - Automotive Bus Schema
# -------------------------------------------
FetchContent_Declare(automotive-bus-schema
    URL https://github.com/boschglobal/automotive-bus-schema/releases/download/v1.0.2/automotive-bus-schema.tar.gz
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



# Set the project paths
# =====================
set(DSE_NCODEC_SOURCE_DIR ../../../../../dse/ncodec)
set(DSE_NCODEC_SOURCE_FILES
    ${DSE_NCODEC_SOURCE_DIR}/libs/automotive-bus/codec.c
    ${DSE_NCODEC_SOURCE_DIR}/libs/automotive-bus/frame_can_fbs.c
    ${DSE_NCODEC_SOURCE_DIR}/codec.c
)
set(DSE_NCODEC_INCLUDE_DIR "${DSE_NCODEC_SOURCE_DIR}/../..")
set(DSE_NCODEC_LIBS_INCLUDE_DIR "${DSE_NCODEC_SOURCE_DIR}/libs")



# Targets
# =======

# Target - Codec
# --------------
add_executable(test_codec
    __test__.c
    test_codec.c
    test_can_fbs.c
    stream.c
    ${DSE_NCODEC_SOURCE_FILES}
    ${FLATCC_SOURCE_FILES}
)
target_include_directories(test_codec
    PRIVATE
        ${DSE_NCODEC_INCLUDE_DIR}
        ${DSE_NCODEC_LIBS_INCLUDE_DIR}
        ${FLATCC_INCLUDE_DIR}
        ${SCHEMAS_SOURCE_DIR}
        ./
)
target_link_libraries(test_codec
    PRIVATE
        cmocka
        dl
        m
)
install(TARGETS test_codec)
