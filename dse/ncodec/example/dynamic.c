// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <dse/ncodec/codec.h>


#define UNUSED(x) ((void)x)


static void* __handle = NULL;


int ncodec_load(const char* filename, const char* hint)
{
    UNUSED(hint);

    __handle = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
    if (__handle == NULL) {
        printf("Error opening library: %s\n", dlerror());
        return -1;
    }
    return 0;
}


NCODEC* ncodec_open(const char* mime_type, NCodecStreamVTable* stream)
{
    if (__handle == NULL) {
        errno = ELIBACC;
        return NULL;
    }
    if (stream == NULL) {
        errno = EINVAL;
        return NULL;
    }

    errno = 0;
    NCodecCreate* create_func = dlsym(__handle, "ncodec_create");
    if (create_func) {
        NCODEC* nc = create_func(mime_type);
        if (nc) {
            NCodecInstance* _nc = (NCodecInstance*)nc;
            _nc->stream = stream;
            return nc;
        } else {
            if (errno == 0) errno = ENODATA;
        }
    } else {
        if (errno == 0) errno = ENOENT;
    }

    return NULL;
}
