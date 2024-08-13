// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <testing.h>
#include <stddef.h>
#include <dse/clib/collections/hashmap.h>
#include <dse/ncodec/codec.h>
#include <bus_topology.h>


#define STRING_MESSAGE  "hello"
#define ASCII85_MESSAGE "BOu!rDZ"


extern char*    ascii85_encode(const uint8_t* data, size_t len);
extern uint8_t* ascii85_decode(const char* source, size_t* len);


int test_setup(void** state)
{
    BT_Mock* mock = calloc(1, sizeof(BT_Mock));
    assert_non_null(mock);
    mock->xml_path = "../../example/modelDescription.xml";
    mock->mime_type = "application/x-automotive-bus; "
                      "interface=stream;type=frame;bus=can;schema=fbs;"
                      "bus_id=1;node_id=2;interface_id=3";
    mock->bus_id = "1";
    mock->stream = stream_create();
    mock->ncodec = ncodec_open(mock->mime_type, mock->stream);
    *state = mock;
    return 0;
}


int test_teardown(void** state)
{
    BT_Mock* mock = *state;
    if (mock) {
        free(mock->stream);
        free(mock);
    }
    return 0;
}


void test_bt_create_destroy(void** state)
{
    BT_Mock* mock = *state;

    BusTopology* bt = bus_topology_create(mock->xml_path);
    assert_non_null(bt);
    assert_non_null(bt->model_xml_path);
    assert_string_equal(bt->model_xml_path, mock->xml_path);
    assert_int_equal(hashmap_number_keys(bt->bus_ncodec), 0);
    assert_int_equal(hashmap_number_keys(bt->rx_vr_index), 0);
    assert_int_equal(hashmap_number_keys(bt->tx_vr_index), 0);
    assert_int_equal(hashmap_number_keys(bt->encode_func), 0);
    assert_int_equal(hashmap_number_keys(bt->decode_func), 0);

    hashmap_set(&bt->bus_ncodec, "ext_free", mock->ncodec);
    hashmap_set_alt(&bt->rx_vr_index, "free", malloc(1));
    hashmap_set_alt(&bt->tx_vr_index, "free", malloc(1));
    hashmap_set_alt(&bt->encode_func, "free", malloc(1));
    hashmap_set_alt(&bt->decode_func, "free", malloc(1));
    assert_int_equal(hashmap_number_keys(bt->bus_ncodec), 1);
    assert_int_equal(hashmap_number_keys(bt->rx_vr_index), 1);
    assert_int_equal(hashmap_number_keys(bt->tx_vr_index), 1);
    assert_int_equal(hashmap_number_keys(bt->encode_func), 1);
    assert_int_equal(hashmap_number_keys(bt->decode_func), 1);

    bus_topology_destroy(bt);
}


void test_bt_add(void** state)
{
    BT_Mock* mock = *state;

    BusTopology* bt = bus_topology_create(mock->xml_path);
    bus_topology_add(bt, mock->bus_id, mock->ncodec);

    assert_int_equal(hashmap_number_keys(bt->bus_ncodec), 1);
    assert_ptr_equal(hashmap_get(&bt->bus_ncodec, mock->bus_id), mock->ncodec);

    assert_int_equal(hashmap_number_keys(bt->rx_vr_index), 3);
    assert_ptr_equal(hashmap_get(&bt->rx_vr_index, "2"), mock->ncodec);
    assert_ptr_equal(hashmap_get(&bt->rx_vr_index, "4"), mock->ncodec);
    assert_ptr_equal(hashmap_get(&bt->rx_vr_index, "6"), mock->ncodec);

    assert_int_equal(hashmap_number_keys(bt->tx_vr_index), 3);
    assert_ptr_equal(hashmap_get(&bt->tx_vr_index, "3"), mock->ncodec);
    assert_ptr_equal(hashmap_get(&bt->tx_vr_index, "5"), mock->ncodec);
    assert_ptr_equal(hashmap_get(&bt->tx_vr_index, "7"), mock->ncodec);

    assert_int_equal(hashmap_number_keys(bt->encode_func), 3);
    assert_ptr_equal(hashmap_get(&bt->encode_func, "3"), ascii85_encode);
    assert_ptr_equal(hashmap_get(&bt->encode_func, "5"), ascii85_encode);
    assert_ptr_equal(hashmap_get(&bt->encode_func, "7"), ascii85_encode);

    assert_int_equal(hashmap_number_keys(bt->decode_func), 3);
    assert_ptr_equal(hashmap_get(&bt->decode_func, "2"), ascii85_decode);
    assert_ptr_equal(hashmap_get(&bt->decode_func, "4"), ascii85_decode);
    assert_ptr_equal(hashmap_get(&bt->decode_func, "6"), ascii85_decode);

    bus_topology_destroy(bt);
}


void test_bt_rx(void** state)
{
    BT_Mock* mock = *state;
    assert_non_null(mock->ncodec);
    NCodecInstance* ncodec = mock->ncodec;
    assert_non_null(ncodec->stream);
    BufferStream* stream = (BufferStream*)ncodec->stream;

    BusTopology* bt = bus_topology_create(mock->xml_path);
    bus_topology_add(bt, mock->bus_id, mock->ncodec);

    /* Consume string. */
    assert_int_equal(ncodec_tell(mock->ncodec), 0);
    bus_topology_rx(bt, 2, (void*)ASCII85_MESSAGE, strlen(ASCII85_MESSAGE));
    ncodec_seek(mock->ncodec, 0, NCODEC_SEEK_END);
    assert_int_equal(ncodec_tell(mock->ncodec), strlen(STRING_MESSAGE));
    assert_memory_equal(stream->buffer, STRING_MESSAGE, strlen(STRING_MESSAGE));

    /* Consume entire topology. */
    ncodec_truncate(mock->ncodec);
    bus_topology_rx(bt, 2, (void*)ASCII85_MESSAGE, strlen(ASCII85_MESSAGE));
    bus_topology_rx(bt, 4, (void*)ASCII85_MESSAGE, strlen(ASCII85_MESSAGE));
    bus_topology_rx(bt, 6, (void*)ASCII85_MESSAGE, strlen(ASCII85_MESSAGE));
    ncodec_seek(mock->ncodec, 0, NCODEC_SEEK_END);
    assert_int_equal(ncodec_tell(mock->ncodec), strlen(STRING_MESSAGE) * 3);

    bus_topology_destroy(bt);
}


void test_bt_tx(void** state)
{
    BT_Mock* mock = *state;

    BusTopology* bt = bus_topology_create(mock->xml_path);
    bus_topology_add(bt, mock->bus_id, mock->ncodec);

    uint8_t* data = NULL;
    size_t   len = 0;

    /* Setup stream with string. */
    ncodec_truncate(mock->ncodec);
    bus_topology_rx(bt, 2, (void*)ASCII85_MESSAGE, strlen(ASCII85_MESSAGE));

    /* Produce to entire topology. */
    data = NULL;
    len = 0;
    bus_topology_tx(bt, 3, &data, &len);
    assert_int_equal(len, strlen(ASCII85_MESSAGE));
    assert_memory_equal(data, ASCII85_MESSAGE, strlen(ASCII85_MESSAGE));

    data = NULL;
    len = 0;
    bus_topology_tx(bt, 5, &data, &len);
    assert_int_equal(len, strlen(ASCII85_MESSAGE));
    assert_memory_equal(data, ASCII85_MESSAGE, strlen(ASCII85_MESSAGE));

    data = NULL;
    len = 0;
    bus_topology_tx(bt, 7, &data, &len);
    assert_int_equal(len, strlen(ASCII85_MESSAGE));
    assert_memory_equal(data, ASCII85_MESSAGE, strlen(ASCII85_MESSAGE));

    bus_topology_destroy(bt);
}


void test_bt_reset(void** state)
{
    BT_Mock* mock = *state;

    BusTopology* bt = bus_topology_create(mock->xml_path);
    bus_topology_add(bt, mock->bus_id, mock->ncodec);

    uint8_t* data = NULL;
    size_t   len = 0;

    /* Setup stream with string. */
    bus_topology_reset(bt);
    bus_topology_rx(bt, 2, (void*)ASCII85_MESSAGE, strlen(ASCII85_MESSAGE));

    /* Produce to entire topology. */
    data = NULL;
    len = 0;
    bus_topology_tx(bt, 3, &data, &len);
    bus_topology_tx(bt, 5, &data, &len);
    bus_topology_tx(bt, 7, &data, &len);
    assert_int_equal(hashmap_number_keys(bt->free_list), 3);

    /* Free used memory. */
    bus_topology_reset(bt);
    assert_int_equal(hashmap_number_keys(bt->free_list), 0);

    /* Produce to entire topology, destroy will free. */
    bus_topology_tx(bt, 3, &data, &len);
    bus_topology_tx(bt, 5, &data, &len);
    bus_topology_tx(bt, 7, &data, &len);
    assert_int_equal(hashmap_number_keys(bt->free_list), 3);

    bus_topology_destroy(bt);
}


int run_bus_topology_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest _tests[] = {
        cmocka_unit_test_setup_teardown(test_bt_create_destroy, s, t),
        cmocka_unit_test_setup_teardown(test_bt_add, s, t),
        cmocka_unit_test_setup_teardown(test_bt_rx, s, t),
        cmocka_unit_test_setup_teardown(test_bt_tx, s, t),
        cmocka_unit_test_setup_teardown(test_bt_reset, s, t),
    };

    return cmocka_run_group_tests_name("BUS_TOPOLOGY", _tests, NULL, NULL);
}
