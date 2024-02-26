// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <testing.h>
#include <stddef.h>
#include <dse/ncodec/codec.h>
#include <automotive-bus/codec.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))


extern void             free_codec(ABCodecInstance* nc);
extern char*            trim(char* s);
extern int              codec_config(NCODEC* nc, NCodecConfigItem item);
extern NCodecConfigItem codec_stat(NCODEC* nc, int* index);
extern NCODEC*          ncodec_create(const char* mime_type);
extern void             codec_close(NCODEC* nc);
extern int              can_write(NCODEC* nc, NCodecMessage* msg);
extern int              can_read(NCODEC* nc, NCodecMessage* msg);
extern int              can_flush(NCODEC* nc);
extern int              can_truncate(NCODEC* nc);

/* Stream mock. */
extern NCodecStreamVTable mem_stream;
extern NCODEC* ncodec_open(const char* mime_type, NCodecStreamVTable* stream);


static void _adjust_node_id(NCODEC* nc, const char* node_id)
{
    ncodec_config(nc, (struct NCodecConfigItem){
                          .name = "node_id",
                          .value = node_id,
                      });
}


typedef struct Mock {
    ABCodecInstance* nc;
} Mock;


static int test_setup(void** state)
{
    Mock* mock = calloc(1, sizeof(Mock));
    assert_non_null(mock);
    mock->nc = calloc(1, sizeof(ABCodecInstance));
    assert_non_null(mock->nc);

    *state = mock;
    return 0;
}


static int test_teardown(void** state)
{
    Mock* mock = *state;
    if (mock && mock->nc) free(mock->nc);
    if (mock) free(mock);

    return 0;
}


void test_trim(void** state)
{
    Mock* mock = *state;
    UNUSED(mock);
    char* s1;
    char* orig_s1;

    orig_s1 = s1 = strdup("  foo bar  ");
    s1 = trim(s1);
    assert_string_equal(s1, "foo bar");
    free(orig_s1);
};


typedef struct codec_config_tc {
    const char* name;
    const char* value;
    uint8_t     int_value;
    size_t      offset_value;
    size_t      offset_int_value;
    /* For negative checks. */
    const char* check_value;
    uint8_t     check_int_value;
} codec_config_tc;

void test_codec_config(void** state)
{
    Mock*            mock = *state;
    ABCodecInstance* nc = mock->nc;

    codec_config_tc tc[] = {
        /* Good values. */
        { .name = "interface",
            .value = "IF",
            .offset_value = offsetof(ABCodecInstance, interface),
            .offset_int_value = 0 },
        { .name = "type",
            .value = "TY",
            .offset_value = offsetof(ABCodecInstance, type),
            .offset_int_value = 0 },
        { .name = "bus",
            .value = "BU",
            .offset_value = offsetof(ABCodecInstance, bus),
            .offset_int_value = 0 },
        { .name = "schema",
            .value = "SC",
            .offset_value = offsetof(ABCodecInstance, schema),
            .offset_int_value = 0 },
        { .name = "bus_id",
            .value = "1",
            .int_value = 1,
            .offset_value = offsetof(ABCodecInstance, bus_id_str),
            .offset_int_value = offsetof(ABCodecInstance, bus_id) },
        { .name = "node_id",
            .value = "2",
            .int_value = 2,
            .offset_value = offsetof(ABCodecInstance, node_id_str),
            .offset_int_value = offsetof(ABCodecInstance, node_id) },
        { .name = "interface_id",
            .value = "3",
            .int_value = 3,
            .offset_value = offsetof(ABCodecInstance, interface_id_str),
            .offset_int_value = offsetof(ABCodecInstance, interface_id) },
        /* Additional values, check for memory leaks. */
        { .name = "interface",
            .value = "IF2",
            .offset_value = offsetof(ABCodecInstance, interface),
            .offset_int_value = 0 },
        { .name = "type",
            .value = "TY2",
            .offset_value = offsetof(ABCodecInstance, type),
            .offset_int_value = 0 },
        { .name = "bus",
            .value = "BU2",
            .offset_value = offsetof(ABCodecInstance, bus),
            .offset_int_value = 0 },
        { .name = "schema",
            .value = "SC2",
            .offset_value = offsetof(ABCodecInstance, schema),
            .offset_int_value = 0 },
        { .name = "bus_id",
            .value = "4",
            .int_value = 4,
            .offset_value = offsetof(ABCodecInstance, bus_id_str),
            .offset_int_value = offsetof(ABCodecInstance, bus_id) },
        { .name = "node_id",
            .value = "5",
            .int_value = 5,
            .offset_value = offsetof(ABCodecInstance, node_id_str),
            .offset_int_value = offsetof(ABCodecInstance, node_id) },
        { .name = "interface_id",
            .value = "6",
            .int_value = 6,
            .offset_value = offsetof(ABCodecInstance, interface_id_str),
            .offset_int_value = offsetof(ABCodecInstance, interface_id) },
        /* Bad integer values. */
        { .name = "bus_id",
            .value = "seven",
            .int_value = 0,
            .offset_int_value = offsetof(ABCodecInstance, bus_id) },
        { .name = "node_id",
            .value = "eight",
            .int_value = 0,
            .offset_int_value = offsetof(ABCodecInstance, node_id) },
        { .name = "interface_id",
            .value = "nine",
            .int_value = 0,
            .offset_int_value = offsetof(ABCodecInstance, interface_id) },
        /* Null values*/
    };

    size_t tc_count = 0;
    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        char**   _ov = (void*)nc + tc[i].offset_value;
        uint8_t* _oiv = (void*)nc + tc[i].offset_int_value;

        codec_config((void*)nc, (struct NCodecConfigItem){
                                    .name = tc[i].name,
                                    .value = tc[i].value,
                                });

        if (tc[i].offset_value) {
            assert_string_equal(*_ov, tc[i].value);
        }
        if (tc[i].offset_int_value) {
            assert_int_equal(*_oiv, tc[i].int_value);
        }
        tc_count++;
    }
    assert_int_equal(tc_count, ARRAY_SIZE(tc));

    free_codec(nc);
}


typedef struct codec_stat_tc {
    int         index;
    const char* name;
    const char* value;
} codec_stat_tc;

void test_codec_stat(void** state)
{
    Mock*            mock = *state;
    ABCodecInstance* nc = mock->nc;

    codec_stat_tc tc[] = {
        /* Good values. */
        { .index = 0, .name = "interface", .value = "IF" },
        { .index = 1, .name = "type", .value = "TY" },
        { .index = 2, .name = "bus", .value = "BU" },
        { .index = 3, .name = "schema", .value = "SC" },
        { .index = 4, .name = "bus_id", .value = "1" },
        { .index = 5, .name = "node_id", .value = "2" },
        { .index = 6, .name = "interface_id", .value = "3" },
        { .index = -1, .name = "foo", .value = "bar" },
    };

    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        codec_config((void*)nc, (struct NCodecConfigItem){
                                    .name = tc[i].name,
                                    .value = tc[i].value,
                                });
    }

    size_t tc_count = 0;
    int    index = 0;
    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        NCodecConfigItem ci = codec_stat((void*)nc, &index);
        assert_int_equal(index, tc[i].index);
        if (tc[i].index >= 0) {
            assert_string_equal(ci.name, tc[i].name);
            assert_string_equal(ci.value, tc[i].value);
        } else {
            assert_null(ci.name);
            assert_null(ci.value);
        }

        index++;
        tc_count++;
    }
    assert_int_equal(tc_count, ARRAY_SIZE(tc));

    free_codec(nc);
}


void test_ncodec_create_close(void** state)
{
    UNUSED(state);

    codec_stat_tc tc[] = {
        /* Good values. */
        { .index = 0, .name = "interface", .value = "stream" },
        { .index = 1, .name = "type", .value = "frame" },
        { .index = 2, .name = "bus", .value = "can" },
        { .index = 3, .name = "schema", .value = "fbs" },
        { .index = 4, .name = "bus_id", .value = "1" },
        { .index = 5, .name = "node_id", .value = "2" },
        { .index = 6, .name = "interface_id", .value = "3" },
    };

    /* Create the codec instance. */
    const char*     mime_type = "application/x-automotive-bus; "
                                "interface=stream;type=frame;bus=can;schema=fbs;"
                                "bus_id=1;node_id=2;interface_id=3";
    NCodecInstance* nc = (NCodecInstance*)ncodec_create(mime_type);
    assert_non_null(nc);
    assert_string_equal(nc->mime_type, mime_type);
    assert_ptr_equal(nc->codec.config, codec_config);
    assert_ptr_equal(nc->codec.stat, codec_stat);
    assert_ptr_equal(nc->codec.write, can_write);
    assert_ptr_equal(nc->codec.read, can_read);
    assert_ptr_equal(nc->codec.flush, can_flush);
    assert_ptr_equal(nc->codec.truncate, can_truncate);
    assert_ptr_equal(nc->codec.close, codec_close);

    /* Check the values. */
    size_t tc_count = 0;
    int    index = 0;
    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        NCodecConfigItem ci = codec_stat((void*)nc, &index);
        assert_int_equal(index, tc[i].index);
        assert_string_equal(ci.name, tc[i].name);
        assert_string_equal(ci.value, tc[i].value);
        index++;
        tc_count++;
    }
    assert_int_equal(tc_count, ARRAY_SIZE(tc));

    codec_close((void*)nc);
}


void test_ncodec_create_failon_mime(void** state)
{
    UNUSED(state);

    const char* tc[] = {
        "application/bus",
        "application/x-automotive-bus; "
        "interface=stream;type=frame;bus=can;schema=FOO",
        "application/x-automotive-bus; "
        "interface=stream;type=frame;bus=FOO;schema=fbs",
        "application/x-automotive-bus; "
        "interface=stream;type=FOO;bus=can;schema=fbs",
        "application/x-automotive-bus; "
        "interface=FOO;type=frame;bus=can;schema=fbs",

        "application/x-automotive-bus; interface=stream;type=frame;bus=can",
        "application/x-automotive-bus; interface=stream;type=frame;schema=fbs",
        "application/x-automotive-bus; interface=stream;bus=can;schema=fbs",
        "application/x-automotive-bus; type=frame;bus=can;schema=fbs",

        "application/x-automotive-bus; interface=stream",
        "application/x-automotive-bus; type=frame",
        "application/x-automotive-bus; bus=can",
        "application/x-automotive-bus; schema=fbs",

        "application/x-automotive-bus ;type=frame;bus=can;schema=fbs",
        "application/x-automotive-bus ;type=frame ; bus = can; schema = fbs ",
        "application/x-automotive-bus ;type=frame ; bus = can;schema =fbs ",
    };

    /* Test with non-correct/supported MIMEtypes. */
    size_t tc_count = 0;
    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        NCODEC* nc = ncodec_create(tc[i]);
        assert_null(nc);
        tc_count++;
    }
    assert_int_equal(tc_count, ARRAY_SIZE(tc));
}


void test_ncodec_call_sequence(void** state)
{
    UNUSED(state);

    /* Create the codec instance. */
    const char* mime_type = "application/x-automotive-bus; "
                            "interface=stream;type=frame;bus=can;schema=fbs;"
                            "bus_id=1;node_id=2;interface_id=3";
    NCODEC*     nc = ncodec_open(mime_type, (void*)&mem_stream);
    assert_non_null(nc);
    NCodecInstance* _nc = (NCodecInstance*)nc;
    assert_non_null(_nc);
    assert_non_null(_nc->stream);
    assert_non_null(_nc->stream->seek);
    assert_string_equal(_nc->mime_type, mime_type);

    /* Initial conditions. */
    const char* greeting = "Hello World";
    assert_int_equal(0, ncodec_seek(nc, 0, NCODEC_SEEK_END));
    assert_int_equal(0, ncodec_tell(nc));
    _adjust_node_id(nc, "42");
    size_t len = ncodec_write(nc, &(struct NCodecCanMessage){ .frame_id = 42,
                                      .buffer = (uint8_t*)greeting,
                                      .len = strlen(greeting) });
    assert_int_equal(strlen(greeting), len);
    ncodec_flush(nc);
    _adjust_node_id(nc, "2");
#define EXPECT_POS 0x66
    assert_int_equal(EXPECT_POS, ncodec_tell(nc));

    /* Read then Write. */
    for (int i = 0; i < 5; i++) {
        /* Seek - position to start of stream. */
        assert_int_equal(EXPECT_POS, ncodec_seek(nc, 0, NCODEC_SEEK_END));
        ncodec_seek(nc, 0, NCODEC_SEEK_SET);
        assert_int_equal(0, ncodec_tell(nc));
        /* Read - consume the stream. */
        NCodecCanMessage msg = {};
        size_t           len = ncodec_read(nc, &msg);
        assert_int_equal(strlen(greeting), len);
        /* Truncate - reset the stream. */
        ncodec_truncate(nc);
        assert_int_equal(0, ncodec_tell(nc));
        assert_int_equal(0, ncodec_seek(nc, 0, NCODEC_SEEK_END));
        assert_int_equal(0, ncodec_tell(nc));
        /* Write and Flush - write to stream (spoof node_id). */
        _adjust_node_id(nc, "42");
        len = ncodec_write(nc, &(struct NCodecCanMessage){ .frame_id = 42,
                                   .buffer = (uint8_t*)greeting,
                                   .len = strlen(greeting) });
        assert_int_equal(strlen(greeting), len);
        ncodec_flush(nc);
        _adjust_node_id(nc, "2");
        assert_int_equal(EXPECT_POS, ncodec_tell(nc));
        assert_int_equal(EXPECT_POS, ncodec_seek(nc, 0, NCODEC_SEEK_END));
    }

    /* Close the codec. */
    ncodec_close((void*)nc);
}


int run_codec_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest codec_tests[] = {
        cmocka_unit_test_setup_teardown(test_trim, s, t),
        cmocka_unit_test_setup_teardown(test_codec_config, s, t),
        cmocka_unit_test_setup_teardown(test_codec_stat, s, t),
        cmocka_unit_test_setup_teardown(test_ncodec_create_close, s, t),
        cmocka_unit_test_setup_teardown(test_ncodec_create_failon_mime, s, t),
        cmocka_unit_test_setup_teardown(test_ncodec_call_sequence, s, t),
    };

    return cmocka_run_group_tests_name("CODEC", codec_tests, NULL, NULL);
}
