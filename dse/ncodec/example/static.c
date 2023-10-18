// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/ncodec/codec.h>


#define UNUSED(x) ((void)x)


int ncodec_load(const char* filename, const char* hint)
{
    UNUSED(filename);
    UNUSED(hint);

    return 0;
}

NCODEC* ncodec_open(const char* mime_type, NCodecStreamVTable* stream)
{
    NCODEC* nc = ncodec_create(mime_type);
    if (nc) {
        NCodecInstance* _nc = (NCodecInstance*)nc;
        _nc->stream = stream;
    }
    return nc;
}
