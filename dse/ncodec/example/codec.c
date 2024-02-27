// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>
#include <dse/ncodec/codec.h>


#define UNUSED(x) ((void)x)
#define CODEC     "application/x-codec-example"


/* Declare an extension to the NCodecInstance type. */
typedef struct __codec {
    NCodecInstance c;
    const char*    name;
} __codec;


int32_t codec_config(NCODEC* nc, NCodecConfigItem item)
{
    if (nc == NULL) return -ENOSTR;

    if (strcmp(item.name, "name") == 0) {
        __codec* _nc = (__codec*)nc;
        _nc->name = item.value;
    }
    return 0;
}

NCodecConfigItem codec_stat(NCODEC* nc, int32_t* index)
{
    if (nc && index && (*index == 0)) {
        __codec* _nc = (__codec*)nc;
        return (struct NCodecConfigItem){
            .name = "name",
            .value = _nc->name,
        };
    }
    /* No more config, return empty object. */
    if (index) *index = -1;
    return (struct NCodecConfigItem){};
}

int32_t codec_write(NCODEC* nc, NCodecMessage* msg)
{
    if (nc == NULL) return -ENOSTR;
    if (msg == NULL) return -EINVAL;
    NCodecCanMessage* _msg = (NCodecCanMessage*)msg;

    __codec* _nc = (__codec*)nc;
    _nc->c.stream->write(nc, _msg->buffer, _msg->len);
    return _msg->len;
}

int32_t codec_read(NCODEC* nc, NCodecMessage* msg)
{
    if (nc == NULL) return -ENOSTR;
    if (msg == NULL) return -EINVAL;
    NCodecCanMessage* _msg = (NCodecCanMessage*)msg;

    __codec* _nc = (__codec*)nc;

    /* Read the stream. */
    char*  data = NULL;
    size_t len = 0;
    _nc->c.stream->read(nc, (uint8_t**)&data, &len, NCODEC_POS_NC);
    /* Sneak a peak at the stream buffer length. */
    _nc->c.stream->seek(nc, 0, 42);
    size_t buffer_len = _nc->c.stream->tell(nc);
    _nc->c.stream->seek(nc, 0, NCODEC_SEEK_SET);
    /* Construct the response. */
    strncat(data, " says ", buffer_len - strlen(data));
    strncat(data, _nc->name, buffer_len);
    _msg->len = strlen(data);
    /* Return the message (buffer owned by stream, caller must duplicate). */
    _msg->buffer = (uint8_t*)data;
    return _msg->len;
}

int32_t codec_flush(NCODEC* nc)
{
    if (nc == NULL) return -ENOSTR;

    return 0;
}

void codec_close(NCODEC* nc)
{
    UNUSED(nc);
}

NCODEC* ncodec_create(const char* mime_type)
{
    /* Define a single codec instance. */
    static __codec codec_inst;

    if (strncmp(mime_type, CODEC, strlen(CODEC)) != 0) return NULL;

    codec_inst.c.mime_type = mime_type;
    codec_inst.c.codec = (struct NCodecVTable){
        .config = codec_config,
        .stat = codec_stat,
        .write = codec_write,
        .read = codec_read,
        .flush = codec_flush,
        .close = codec_close,
    };
    return (void*)&codec_inst;
}
