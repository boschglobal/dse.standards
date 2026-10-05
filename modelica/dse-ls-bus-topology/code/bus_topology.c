// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <bus_topology.h>


#define HASH_KEY_LEN (10 + 1)


/**
bus_topology_create
===================

Create a `BusTopology` object, initialised and ready for operation.

Parameters
----------
model_xml_path (const char*)
: Path of `modelDescription.xml` file to be parsed.

Returns
-------
BusTopology*
: An initialised `BusTopology` object.

*/
BusTopology* bus_topology_create(const char* model_xml_path)
{
    assert(model_xml_path);

    BusTopology* bt = calloc(1, sizeof(BusTopology));
    bt->model_xml_path = model_xml_path;
    hashmap_init(&bt->bus_ncodec);
    hashmap_init(&bt->rx_vr_index);
    hashmap_init(&bt->tx_vr_index);
    hashmap_init(&bt->encode_func);
    hashmap_init(&bt->decode_func);
    hashmap_init(&bt->free_list);

    return bt;
}


/**
bus_topology_add
================

Add a Bus Topology for the specified `bus_id` and `ncodec`.

Parameters
----------
bt (BusTopology*)
: A `BusTopology` object.

bus_id (const char*)
: The name of the tool annotations container.

ncodec (void*)
: The name of the annotation to return.
*/
void bus_topology_add(BusTopology* bt, const char* bus_id, void* ncodec)
{
    hashmap_set(&bt->bus_ncodec, bus_id, ncodec);
    parse_bus_topology(
        bt->model_xml_path, bus_id, ncodec, &bt->rx_vr_index, &bt->tx_vr_index);
    parse_binary_to_text(
        bt->model_xml_path, &bt->encode_func, &bt->decode_func);
}


/**
bus_topology_rx
===============

For the provided `BusTopology` and `vr` (FMI Value reference), if an index
entry exists, then data is copied from the Variable to the `stream` object
of the indexed Network Codec.

If a `binary-to-text` encoder is configured, the text is decoded during the
copy operation.

Parameters
----------
bt (BusTopology*)
: A `BusTopology` object.

vr (uint32_t)
: FMI Value Reference for the FMI Variable being exchanged (from).

data (uint8_t*)
: Data being exchanged.

len (size_t)
: Length of the data being exchanged.
*/
void bus_topology_rx(BusTopology* bt, uint32_t vr, uint8_t* data, size_t len)
{
    assert(bt);

    /* Locate the codec. */
    char key[HASH_KEY_LEN];
    snprintf(key, HASH_KEY_LEN, "%u", vr);
    NCodecInstance* ncodec = hashmap_get(&bt->rx_vr_index, key);
    if (ncodec == NULL || ncodec->stream == NULL) return;
    NCodecStreamVTable* stream = (NCodecStreamVTable*)ncodec->stream;

    /* Write (append) the RX data directly to the underlying stream. */
    DecodeFunc df = hashmap_get(&bt->decode_func, key);
    if (df) {
        /* Use encoder if configured. */
        data = df((char*)data, &len);
        stream->seek((NCODEC*)ncodec, 0, NCODEC_SEEK_END);
        stream->write((NCODEC*)ncodec, data, len);
        stream->seek((NCODEC*)ncodec, 0, NCODEC_SEEK_SET);
        free(data);
    } else {
        stream->seek((NCODEC*)ncodec, 0, NCODEC_SEEK_END);
        stream->write((NCODEC*)ncodec, data, len);
        stream->seek((NCODEC*)ncodec, 0, NCODEC_SEEK_SET);
    }
}


/**
bus_topology_tx
===============

For the provided `BusTopology` and `vr` (FMI Value reference), if an index
entry exists, then data is copied from the `stream` object of the indexed
Network Codec to the Variable .

If a `binary-to-text` encoder is configured, the text is encoded during the
copy operation.

Parameters
----------
bt (BusTopology*)
: A `BusTopology` object.

vr (uint32_t)
: FMI Value Reference for the FMI Variable being exchanged (towards).

data (uint8_t*)
: (out) Data being exchanged.

len (size_t)
: (out) Length of the data being exchanged.
*/
void bus_topology_tx(BusTopology* bt, uint32_t vr, uint8_t** data, size_t* len)
{
    assert(bt);
    bt->reset_called = false; /* Indicate that reset is pending. */

    /* Locate the codec. */
    char key[HASH_KEY_LEN];
    snprintf(key, HASH_KEY_LEN, "%u", vr);
    NCodecInstance* ncodec = hashmap_get(&bt->tx_vr_index, key);
    if (ncodec == NULL || ncodec->stream == NULL) return;
    NCodecStreamVTable* stream = (NCodecStreamVTable*)ncodec->stream;

    /* Read the TX data directly from the underlying stream. */
    uint8_t* _ = NULL;
    uint8_t* tx_data = NULL;
    size_t   tx_len = 0;
    stream->seek((NCODEC*)ncodec, 0, NCODEC_SEEK_SET);
    stream->read((NCODEC*)ncodec, &_, &tx_len, NCODEC_POS_UPDATE);
    tx_data = malloc(tx_len);
    memcpy(tx_data, _, tx_len);

    /* Return the data. */
    EncodeFunc ef = hashmap_get(&bt->encode_func, key);
    if (ef) {
        /* Use encoder if configured. */
        void* _ = tx_data;
        tx_data = (uint8_t*)ef(_, tx_len);
        tx_len = strlen((char*)tx_data);
        free(_);
    }
    *data = tx_data;
    *len = tx_len;

    /* Save reference for later free. */
    snprintf(key, HASH_KEY_LEN, "%lu", bt->free_list.used_nodes);
    hashmap_set_alt(&bt->free_list, key, tx_data);
}


static int _reset_ncodec(void* map_item, void* data)
{
    UNUSED(data);
    ncodec_truncate(map_item);
    return 0;
}


/**
bus_topology_reset
==================

Reset the underlying binary streams and free any memory allocated for
the FMI Variable interface (e.g. during TX operations).

This method is called at the beginning of a Rx cycle. Internally it maintains
state so that the resources are only reset once-per-Rx-cycle.

Parameters
----------
bt (BusTopology*)
: A `BusTopology` object.
*/
void bus_topology_reset(BusTopology* bt)
{
    assert(bt);
    if (bt->reset_called) return;

    hashmap_iterator(&bt->bus_ncodec, _reset_ncodec, true, NULL);
    hashmap_clear(&bt->free_list);
    bt->reset_called = true;
}


static int _destroy_ncodec(void* map_item, void* data)
{
    UNUSED(data);
    ncodec_close(map_item);
    return 0;
}


/**
bus_topology_destroy
===================

Destroy the `BusTopology` object and all related resources.

Parameters
----------
bt (BusTopology*)
: A `BusTopology` object.
*/
void bus_topology_destroy(BusTopology* bt)
{
    if (bt == NULL) return;

    hashmap_iterator(&bt->bus_ncodec, _destroy_ncodec, true, NULL);
    hashmap_destroy(&bt->bus_ncodec);
    hashmap_destroy(&bt->rx_vr_index);
    hashmap_destroy(&bt->tx_vr_index);
    hashmap_destroy(&bt->encode_func);
    hashmap_destroy(&bt->decode_func);
    hashmap_destroy(&bt->free_list);

    free(bt);
}
