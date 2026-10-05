# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(test_bus-topology)


# Targets
# =======

# Target - Test
# --------------
add_executable(test
    __test__.c
    test_bus_topology.c
    test_parser.c
    test_ncodec.c
)
target_include_directories(test
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
        ${DSE_NCODEC_INCLUDE_DIR}
        ./
        ../
)
target_link_libraries(test
    PUBLIC
        bus_topology
        ncodec
    PRIVATE
        xml
        cmocka
        dl
        m
)
install(TARGETS test)
