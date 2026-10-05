// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <testing.h>
#include <stddef.h>
#include <dse/clib/collections/hashmap.h>
#include <bus_topology.h>


extern char*    ascii85_encode(const uint8_t* data, size_t len);
extern uint8_t* ascii85_decode(const char* source, size_t* len);


typedef struct Mock {
} Mock;


static int test_setup(void** state)
{
    Mock* mock = calloc(1, sizeof(Mock));
    assert_non_null(mock);
    *state = mock;
    return 0;
}


static int test_teardown(void** state)
{
    Mock* mock = *state;
    free(mock);
    return 0;
}


void test_xml_parse_bus_topology(void** state)
{
    Mock* mock = *state;
    UNUSED(mock);

    HashMap bus_map;
    HashMap rx_bus_index;
    HashMap tx_bus_index;
    hashmap_init(&bus_map);
    hashmap_init(&rx_bus_index);
    hashmap_init(&tx_bus_index);

    /* Setup a stub bus object. */
    hashmap_set_alt(&bus_map, "1", calloc(1, sizeof(int)));
    void* bus_1_object = hashmap_get(&bus_map, "1");
    assert_non_null(bus_1_object);

    parse_bus_topology("../../example/modelDescription.xml", "1", bus_1_object,
        &rx_bus_index, &tx_bus_index);

    // RX
    assert_int_equal(hashmap_number_keys(rx_bus_index), 3);
    assert_ptr_equal(hashmap_get(&rx_bus_index, "2"), bus_1_object);
    assert_ptr_equal(hashmap_get(&rx_bus_index, "4"), bus_1_object);
    assert_ptr_equal(hashmap_get(&rx_bus_index, "6"), bus_1_object);

    // TX
    assert_int_equal(hashmap_number_keys(tx_bus_index), 3);
    assert_int_equal(hashmap_get(&tx_bus_index, "3"), bus_1_object);
    assert_int_equal(hashmap_get(&tx_bus_index, "5"), bus_1_object);
    assert_int_equal(hashmap_get(&tx_bus_index, "7"), bus_1_object);

    hashmap_destroy(&bus_map);
    hashmap_destroy(&rx_bus_index);
    hashmap_destroy(&tx_bus_index);
}


void test_xml_parse_binary_to_text(void** state)
{
    Mock* mock = *state;
    UNUSED(mock);

    HashMap encode_func;
    HashMap decode_func;
    hashmap_init(&encode_func);
    hashmap_init(&decode_func);

    parse_binary_to_text(
        "../../example/modelDescription.xml", &encode_func, &decode_func);

    // Encode
    assert_int_equal(hashmap_number_keys(encode_func), 3);
    assert_ptr_equal(hashmap_get(&encode_func, "3"), ascii85_encode);
    assert_ptr_equal(hashmap_get(&encode_func, "5"), ascii85_encode);
    assert_ptr_equal(hashmap_get(&encode_func, "7"), ascii85_encode);

    // Decode
    assert_int_equal(hashmap_number_keys(decode_func), 3);
    assert_int_equal(hashmap_get(&decode_func, "2"), ascii85_decode);
    assert_int_equal(hashmap_get(&decode_func, "4"), ascii85_decode);
    assert_int_equal(hashmap_get(&decode_func, "6"), ascii85_decode);

    hashmap_destroy(&encode_func);
    hashmap_destroy(&decode_func);
}


int run_parser_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest _tests[] = {
        cmocka_unit_test_setup_teardown(test_xml_parse_bus_topology, s, t),
        cmocka_unit_test_setup_teardown(test_xml_parse_binary_to_text, s, t),
    };

    return cmocka_run_group_tests_name("PARSER", _tests, NULL, NULL);
}
