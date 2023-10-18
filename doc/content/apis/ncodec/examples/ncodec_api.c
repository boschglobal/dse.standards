// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dse/ncodec/codec.h>


#define MIMETYPE "application/x-codec-example"


extern NCodecStreamVTable example_stream;
extern int                stream_seek(NCODEC* nc, size_t pos, int op);


int main(int argc, char* argv[])
{
    int                rc;
    static const char* greeting = "Hello World";

    if (argc > 1) {
        rc = ncodec_load(argv[1], NULL);
        if (rc) {
            printf("Load failed (rc %d)\n", rc);
            return rc;
        }
    }

    NCODEC* nc = ncodec_open(MIMETYPE, (void*)&example_stream);
    if (nc == NULL) {
        printf("Open failed (errno %d)\n", errno);
        return errno;
    }
    ncodec_config(nc, (struct NCodecConfigItem){
                          .name = "name", .value = "simple network codec" });

    /* Write a message to the Network Codec. */
    ncodec_write(nc, &(struct NCodecMessage){ .frame_id = 42,
                         .buffer = (uint8_t*)greeting,
                         .len = strlen(greeting) });
    ncodec_flush(nc);

    /* Reposition to start of stream. */
    stream_seek(nc, 0, NCODEC_SEEK_SET);

    /* Read the response from the Network Codec. */
    NCodecMessage msg = {};
    rc = ncodec_read(nc, &msg);
    if (rc > 0) {
        printf("Message is: %s\n", (char*)msg.buffer);
    } else {
        printf("There was no message! (reason %d)\n", rc);
    }

    /* Close the Network Codec. */
    ncodec_close(nc);

    return 0;
}
