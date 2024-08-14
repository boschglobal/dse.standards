// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MODELICA_FMI_LS_BUS_TOPOLOGY_CODE_TESTS_TESTING_H_
#define MODELICA_FMI_LS_BUS_TOPOLOGY_CODE_TESTS_TESTING_H_


#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>


/* Wrapping strdup() to get memory checking does not work with libyaml (and
 * perhaps other libs too). Therefore, rather than wrapping we can use the
 * preprocessor to swap strdup with this implementation in DSE code only.
 */
static inline char* dse_strdup_swap(const char* s)
{
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    void*  dup = malloc(len);
    if (dup == NULL) return NULL;
    return (char*)memcpy(dup, s, len);
}
#define strdup dse_strdup_swap


#include <dse/ncodec/codec.h>

typedef struct BT_Mock {
    const char*         xml_path;
    const char*         mime_type;
    const char*         bus_id;
    NCodecStreamVTable* stream;
    void*               ncodec;
} BT_Mock;


#endif  // MODELICA_FMI_LS_BUS_TOPOLOGY_CODE_TESTS_TESTING_H_
