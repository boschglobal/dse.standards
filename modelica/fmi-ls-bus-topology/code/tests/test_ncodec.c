// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <testing.h>
#include <stddef.h>
#include <stdio.h>
#include <dse/ncodec/codec.h>
#include <bus_topology.h>


extern int test_setup(void** state);
extern int test_teardown(void** state);


#define GREETING          "Hello World!"
#define ASCII85_FRAME     ""
#define BUF_NODEID_OFFSET 53


void test_ncodec_write_read(void** state)
{
    BT_Mock* mock = *state;
    uint8_t* data = NULL;
    size_t   len = 0;

    BusTopology* bt = bus_topology_create(mock->xml_path);
    bus_topology_add(bt, mock->bus_id, mock->ncodec);

    /* Write a CAN Frame. */
    int rc =
        ncodec_write(mock->ncodec, &(struct NCodecCanMessage){ .frame_id = 42,
                                       .buffer = (uint8_t*)GREETING,
                                       .len = strlen(GREETING) });
    assert_int_equal(rc, strlen(GREETING));
    len = ncodec_flush(mock->ncodec);
    assert_int_equal(len, 0x66);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x66);

    /* Modify the node_id (ncodec_read filters on node_id). */
    NCodecInstance* ncodec = mock->ncodec;
    BufferStream*   stream = (BufferStream*)ncodec->stream;
    stream->buffer[BUF_NODEID_OFFSET] = 42;
    if (0) {
        uint8_t* buffer = stream->buffer;
        uint32_t buffer_len = len;
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%02x %02x %02x %02x %02x %02x %02x %02x\n", buffer[i + 0],
                buffer[i + 1], buffer[i + 2], buffer[i + 3], buffer[i + 4],
                buffer[i + 5], buffer[i + 6], buffer[i + 7]);
    }

    /* Exchange the Variables (OUT/GET). */
    data = NULL;
    len = 0;
    bus_topology_tx(bt, 3, &data, &len);
    assert_int_equal(len, 0x80);

    /* Reset the stream. */
    ncodec_truncate(mock->ncodec);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x0);

    /* Exchange the Variables (IN/SET). */
    bus_topology_rx(bt, 2, data, len);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x0);
    ncodec_seek(mock->ncodec, 0, NCODEC_SEEK_END);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x66);
    ncodec_seek(mock->ncodec, 0, NCODEC_SEEK_SET);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x0);

    /* Read the CAN Frame. */
    NCodecCanMessage msg = {};
    len = ncodec_read(mock->ncodec, &msg);
    assert_int_equal(len, strlen(GREETING));
    assert_int_equal(msg.len, strlen(GREETING));
    assert_non_null(msg.buffer);
    assert_memory_equal(msg.buffer, GREETING, strlen(GREETING));

    bus_topology_destroy(bt);
}


void test_ncodec_multi_read(void** state)
{
    BT_Mock* mock = *state;
    uint8_t* data = NULL;
    size_t   len = 0;

    BusTopology* bt = bus_topology_create(mock->xml_path);
    bus_topology_add(bt, mock->bus_id, mock->ncodec);

    /* Write a CAN Frame. */
    int rc =
        ncodec_write(mock->ncodec, &(struct NCodecCanMessage){ .frame_id = 42,
                                       .buffer = (uint8_t*)GREETING,
                                       .len = strlen(GREETING) });
    assert_int_equal(rc, strlen(GREETING));
    len = ncodec_flush(mock->ncodec);
    assert_int_equal(len, 0x66);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x66);

    /* Modify the node_id (ncodec_read filters on node_id). */
    NCodecInstance* ncodec = mock->ncodec;
    BufferStream*   stream = (BufferStream*)ncodec->stream;
    stream->buffer[BUF_NODEID_OFFSET] = 42;

    /* Exchange the Variables (OUT/GET). */
    data = NULL;
    len = 0;
    bus_topology_tx(bt, 3, &data, &len);
    assert_int_equal(len, 0x80);
    /* Duplicate the string as bus_topology_reset will free. */
    uint8_t* rx_data = malloc(len + 1);
    memcpy(rx_data, data, len + 1);

    /* Reset the stream. */
    bus_topology_reset(bt);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x0);

    /* Rx several times, ensure stream is not reset again. */
    bus_topology_reset(bt);
    bus_topology_rx(bt, 2, rx_data, len);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x0);
    ncodec_seek(mock->ncodec, 0, NCODEC_SEEK_END);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x66);

    bus_topology_reset(bt);
    bus_topology_rx(bt, 2, rx_data, len);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x0);
    ncodec_seek(mock->ncodec, 0, NCODEC_SEEK_END);
    assert_int_equal(ncodec_tell(mock->ncodec), 0x66 * 2);

    bus_topology_destroy(bt);
    free(rx_data);
}


int run_ncodec_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest _tests[] = {
        cmocka_unit_test_setup_teardown(test_ncodec_write_read, s, t),
        cmocka_unit_test_setup_teardown(test_ncodec_multi_read, s, t),
    };

    return cmocka_run_group_tests_name("NCODEC", _tests, NULL, NULL);
}
