// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <automotive-bus/tests/testing.h>


extern int run_codec_tests(void);
extern int run_can_fbs_tests(void);


int main()
{
    int rc = 0;
    rc |= run_codec_tests();
    rc |= run_can_fbs_tests();
    return rc;
}
