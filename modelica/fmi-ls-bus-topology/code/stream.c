// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <dse/ncodec/codec.h>
#include <bus_topology.h>


size_t stream_read(NCODEC* nc, uint8_t** data, size_t* len, int32_t pos_op)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc == NULL || _nc->stream == NULL) return -ENOSTR;
    if (data == NULL || len == NULL) return -EINVAL;

    BufferStream* _s = (BufferStream*)_nc->stream;
    /* Check EOF. */
    if (_s->pos >= _s->len) {
        *data = NULL;
        *len = 0;
        return 0;
    }
    /* Return buffer, from current pos. */
    *data = &_s->buffer[_s->pos];
    *len = _s->len - _s->pos;
    /* Advance the position indicator. */
    if (pos_op == NCODEC_POS_UPDATE) _s->pos = _s->len;

    return *len;
}

size_t stream_write(NCODEC* nc, uint8_t* data, size_t len)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc == NULL || _nc->stream == NULL) return -ENOSTR;

    BufferStream* _s = (BufferStream*)_nc->stream;

    if ((_s->pos + len) > BUFFER_LEN) return -EMSGSIZE;
    memcpy(&_s->buffer[_s->pos], data, len);
    _s->pos += len;
    if (_s->pos > _s->len) _s->len = _s->pos;
    return len;
}

int64_t stream_seek(NCODEC* nc, size_t pos, int32_t op)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->stream) {
        BufferStream* _s = (BufferStream*)_nc->stream;
        if (op == NCODEC_SEEK_SET) {
            if (pos > _s->len) {
                _s->pos = _s->len;
            } else {
                _s->pos = pos;
            }
        } else if (op == NCODEC_SEEK_CUR) {
            pos = _s->pos + pos;
            if (pos > _s->len) {
                _s->pos = _s->len;
            } else {
                _s->pos = pos;
            }
        } else if (op == NCODEC_SEEK_END) {
            _s->pos = _s->len;
        } else if (op == NCODEC_SEEK_RESET) {
            _s->pos = _s->len = 0;
        } else if (op == 42) {
            _s->pos = _s->len = _s->buffer_len;
        } else {
            return -EINVAL;
        }
        return _s->pos;
    }
    return -ENOSTR;
}

int64_t stream_tell(NCODEC* nc)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->stream) {
        BufferStream* _s = (BufferStream*)_nc->stream;
        return _s->pos;
    }
    return -ENOSTR;
}

int32_t stream_eof(NCODEC* nc)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->stream) {
        BufferStream* _s = (BufferStream*)_nc->stream;
        if (_s->pos < _s->len) return 0;
    }
    return 1;
}

int32_t stream_close(NCODEC* nc)
{
    UNUSED(nc);

    return 0;
}

void* stream_create(void)
{
    BufferStream* stream = calloc(1, sizeof(BufferStream));
    stream->s = (struct NCodecStreamVTable){
        .read = stream_read,
        .write = stream_write,
        .seek = stream_seek,
        .tell = stream_tell,
        .eof = stream_eof,
        .close = stream_close,
    };
    stream->buffer_len = BUFFER_LEN;
    return stream;
}
