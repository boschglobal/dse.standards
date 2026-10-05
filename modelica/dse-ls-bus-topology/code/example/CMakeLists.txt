# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

project(Example_bus-topology)

set(FMI2_INCLUDE_DIR "${DSE_CLIB_SOURCE_DIR}/clib/fmi/fmi2/headers")



# Targets
# =======
add_library(example
    SHARED
        fmu.c
)
target_include_directories(example
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
        ${DSE_NCODEC_INCLUDE_DIR}
        ${FMI2_INCLUDE_DIR}
        ../
)
target_link_libraries(example
    PUBLIC
        bus_topology
        ncodec
    PRIVATE
        xml
        dl
        m
)
install(TARGETS example)
