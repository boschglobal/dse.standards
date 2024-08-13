// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <errno.h>
#include <dse/ncodec/codec.h>


/**
ncodec_open
===========

Open a Network Codec for the corresponding MIMEtype and using the provided
`stream` object.

Parameters
----------
mime_type (const char*)
: The MIMEtype specifier.

stream (NCodecStreamVTable*)
: A `stream` object.

Returns
-------
NCODEC (pointer)
: Object representing the Network Codec.

NULL
: The Network Codec could not be created. Inspect `errno` for more details.

Error Conditions
----------------
Available by inspection of `errno`.

EINVAL
: Stream parameter not valid.

ENODATA
: A Network Codec matching the MIMEtype could not be found.
*/
NCODEC* ncodec_open(const char* mime_type, NCodecStreamVTable* stream)
{
    NCODEC* nc = ncodec_create(mime_type);
    if (nc == NULL || stream == NULL) {
        errno = EINVAL;
        return NULL;
    }
    NCodecInstance* _nc = (NCodecInstance*)nc;
    _nc->stream = stream;
    return nc;
}
