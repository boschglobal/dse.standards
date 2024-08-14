// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MODELICA_FMI_LS_BUS_TOPOLOGY_CODE_BUS_TOPOLOGY_H_
#define MODELICA_FMI_LS_BUS_TOPOLOGY_CODE_BUS_TOPOLOGY_H_


#include <string.h>
#include <dse/clib/collections/hashmap.h>
#include <dse/ncodec/codec.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define BUFFER_LEN    (1024 * 4)


typedef struct BusTopology {
    const char* model_xml_path;
    HashMap     bus_ncodec;
    HashMap     rx_vr_index;
    HashMap     tx_vr_index;
    HashMap     encode_func;
    HashMap     decode_func;
    HashMap     free_list;
    bool        reset_called;
} BusTopology;

typedef struct BufferStream {
    NCodecStreamVTable s;
    uint8_t            buffer[BUFFER_LEN];
    size_t             buffer_len;
    size_t             len;
    size_t             pos;
} BufferStream;

typedef char* (*EncodeFunc)(const uint8_t* data, size_t len);
typedef uint8_t* (*DecodeFunc)(const char* source, size_t* len);


/* bus_topology.c */
BusTopology* bus_topology_create(const char* model_xml_path);
void bus_topology_add(BusTopology* bt, const char* bus_id, void* bus_ncodec);
void bus_topology_rx(BusTopology* bt, uint32_t vr, uint8_t* data, size_t len);
void bus_topology_tx(BusTopology* bt, uint32_t vr, uint8_t** data, size_t* len);
void bus_topology_reset(BusTopology* bt);
void bus_topology_destroy(BusTopology* bt);

/* parser.c */
void parse_bus_topology(const char* model_description_path, const char* bus_id,
    void* bus_object, HashMap* rx, HashMap* tx);
void parse_binary_to_text(const char* model_description_path,
    HashMap* encode_func, HashMap* decode_func);

/* stream.c */
void* stream_create(void);


#endif  // MODELICA_FMI_LS_BUS_TOPOLOGY_CODE_BUS_TOPOLOGY_H_
