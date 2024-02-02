// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dse/ncodec/codec.h>
#include <automotive-bus/codec.h>


#define UNUSED(x) ((void)x)
#define CODEC     "application/x-automotive-bus"


/* interface=stream; type=frame; bus=can; schema=fbs */
extern int can_write(NCODEC* nc, NCodecMessage* msg);
extern int can_read(NCODEC* nc, NCodecMessage* msg);
extern int can_flush(NCODEC* nc);
extern int can_truncate(NCODEC* nc);


char* trim(char* s)
{
    if (s == NULL) return s;

    while (*s && isspace(*s)) s++;
    char* e = s + strlen(s);
    while ((s != e) && isspace(*(e - 1))) e--;
    *e = '\0';
    return s;
}


void free_codec(ABCodecInstance* _nc)
{
    if (_nc == NULL) return;

    if (_nc->interface) free(_nc->interface);
    if (_nc->type) free(_nc->type);
    if (_nc->bus) free(_nc->bus);
    if (_nc->schema) free(_nc->schema);
    if (_nc->bus_id_str) free(_nc->bus_id_str);
    if (_nc->node_id_str) free(_nc->node_id_str);
    if (_nc->interface_id_str) free(_nc->interface_id_str);
    if (_nc->fbs_builder_initalized) flatcc_builder_clear(&_nc->fbs_builder);
}


int codec_config(NCODEC* nc, NCodecConfigItem item)
{
    ABCodecInstance* _nc = (ABCodecInstance*)nc;
    if (_nc == NULL) return -ENOSTR;

    /* Selectors. */
    if (strcmp(item.name, "interface") == 0) {
        if (_nc->interface) free(_nc->interface);
        _nc->interface = strdup(item.value);
        return 0;
    }
    if (strcmp(item.name, "type") == 0) {
        if (_nc->type) free(_nc->type);
        _nc->type = strdup(item.value);
        return 0;
    }
    if (strcmp(item.name, "bus") == 0) {
        if (_nc->bus) free(_nc->bus);
        _nc->bus = strdup(item.value);
        return 0;
    }
    if (strcmp(item.name, "schema") == 0) {
        if (_nc->schema) free(_nc->schema);
        _nc->schema = strdup(item.value);
        return 0;
    }

    /* Parameters. */
    if (strcmp(item.name, "bus_id") == 0) {
        if (_nc->bus_id_str) free(_nc->bus_id_str);
        _nc->bus_id_str = strdup(item.value);
        _nc->bus_id = strtoul(item.value, NULL, 10);
        return 0;
    }
    if (strcmp(item.name, "node_id") == 0) {
        if (_nc->node_id_str) free(_nc->node_id_str);
        _nc->node_id_str = strdup(item.value);
        _nc->node_id = strtoul(item.value, NULL, 10);
        return 0;
    }
    if (strcmp(item.name, "interface_id") == 0) {
        if (_nc->interface_id_str) free(_nc->interface_id_str);
        _nc->interface_id_str = strdup(item.value);
        _nc->interface_id = strtoul(item.value, NULL, 10);
        return 0;
    }

    return -EINVAL;
}


NCodecConfigItem codec_stat(NCODEC* nc, int* index)
{
    ABCodecInstance* _nc = (ABCodecInstance*)nc;
    if ((_nc == NULL) || (index == NULL)) {
        return (struct NCodecConfigItem){};
    }

    const char* name = NULL;
    const char* value = NULL;
    switch (*index) {
    case 0:
        name = "interface";
        value = _nc->interface;
        break;
    case 1:
        name = "type";
        value = _nc->type;
        break;
    case 2:
        name = "bus";
        value = _nc->bus;
        break;
    case 3:
        name = "schema";
        value = _nc->schema;
        break;
    case 4:
        name = "bus_id";
        value = _nc->bus_id_str;
        break;
    case 5:
        name = "node_id";
        value = _nc->node_id_str;
        break;
    case 6:
        name = "interface_id";
        value = _nc->interface_id_str;
        break;
    default:
        *index = -1;
    }

    return (struct NCodecConfigItem){
        .name = name,
        .value = value,
    };
}


void codec_close(NCODEC* nc)
{
    if (nc == NULL) return;

    free_codec((ABCodecInstance*)nc);
    free(nc);
}


NCODEC* ncodec_create(const char* mime_type)
{
    char*            _buf = strdup(mime_type);
    char*            _pos = NULL;
    ABCodecInstance* _nc = NULL;

    /* Check the MIMEtype is correct. */
    char* _codec_name = strtok_r(_buf, "; ", &_pos);
    if (strncmp(_codec_name, CODEC, strlen(CODEC)) != 0) {
        goto create_fail;
    }

    /* Allocate the codec object. */
    _nc = calloc(1, sizeof(ABCodecInstance));
    _nc->c.mime_type = mime_type;

    /* Parse out the remaining parameters from the MIMEtype. */
    char* _param;
    while ((_param = strtok_r(NULL, "; ", &_pos)) != NULL) {
        char* name = _param;
        char* value = strchr(_param, '=');
        if (value) *value++ = '\0';
        if (name && value) {
            codec_config((void*)_nc, (struct NCodecConfigItem){
                                         .name = (const char*)trim(name),
                                         .value = (const char*)trim(value),
                                     });
        }
    }
    free(_buf);
    _buf = NULL;

    /* Guard conditions for this codec.*/
    if (_nc->interface == NULL || _nc->type == NULL || _nc->schema == NULL ||
        _nc->bus == NULL) {
        goto create_fail;
    }
    if (strcmp(_nc->interface, "stream") || strcmp(_nc->type, "frame") ||
        strcmp(_nc->schema, "fbs")) {
        goto create_fail;
    }

    /* Determine which codec implementation to use. */
    if (strcmp(_nc->bus, "can") == 0) {
        _nc->c.codec = (struct NCodecVTable){
            .config = codec_config,
            .stat = codec_stat,
            .write = can_write,
            .read = can_read,
            .flush = can_flush,
            .truncate = can_truncate,
            .close = codec_close,
        };
    } else {
        goto create_fail;
    }

    /* Complete the setup of this codec instance. */
    flatcc_builder_init(&_nc->fbs_builder);
    _nc->fbs_builder.buffer_flags |= flatcc_builder_with_size;
    _nc->fbs_stream_initalized = false;
    _nc->fbs_builder_initalized = true;

    return (void*)_nc;

create_fail:
    if (_buf) free(_buf);
    if (_nc) {
        free_codec(_nc);
        free(_nc);
    }
    return NULL;
}
