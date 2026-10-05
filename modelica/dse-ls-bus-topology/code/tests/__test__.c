// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <testing.h>


extern int run_parser_tests(void);
extern int run_bus_topology_tests(void);
extern int run_ncodec_tests(void);


int main()
{
    int rc = 0;
    rc |= run_parser_tests();
    rc |= run_bus_topology_tests();
    rc |= run_ncodec_tests();
    return rc;
}
