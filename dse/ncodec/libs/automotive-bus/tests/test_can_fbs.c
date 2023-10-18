// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <automotive-bus/tests/testing.h>
#include <stdio.h>
#include <dse/ncodec/codec.h>
#include <automotive-bus/codec.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define BUFFER_LEN    1024


extern void             free_codec(ABCodecInstance* nc);
extern char*            trim(char* s);
extern int              codec_config(NCODEC* nc, NCodecConfigItem item);
extern NCodecConfigItem codec_stat(NCODEC* nc, int* index);
extern NCODEC*          ncodec_create(const char* mime_type);
extern void             codec_close(NCODEC* nc);
extern int              can_write(NCODEC* nc, NCodecMessage* msg);
extern int              can_read(NCODEC* nc, NCodecMessage* msg);
extern int              can_flush(NCODEC* nc);
extern int              stream_seek(NCODEC* nc, size_t pos, int op);
extern int stream_read(NCODEC* nc, uint8_t** data, size_t* len, int pos_op);


typedef struct Mock {
    NCODEC* nc;
} Mock;

extern NCodecStreamVTable mem_stream;


#define MIMETYPE                                                               \
    "application/x-automotive-bus; "                                           \
    "interface=stream;type=frame;bus=can;schema=fbs;"                          \
    "bus_id=1;node_id=2;interface_id=3"


NCODEC* ncodec_open(const char* mime_type, NCodecStreamVTable* stream)
{
    NCODEC* nc = ncodec_create(mime_type);
    if (nc) {
        NCodecInstance* _nc = (NCodecInstance*)nc;
        _nc->stream = stream;
    }
    return nc;
}


static int test_setup(void** state)
{
    Mock* mock = calloc(1, sizeof(Mock));
    assert_non_null(mock);

    mock->nc = (void*)ncodec_open(MIMETYPE, (void*)&mem_stream);
    assert_non_null(mock->nc);

    *state = mock;
    return 0;
}


static int test_teardown(void** state)
{
    Mock* mock = *state;
    if (mock && mock->nc) codec_close((void*)mock->nc);
    if (mock) free(mock);

    return 0;
}


void test_can_fbs_no_stream(void** state)
{
    Mock* mock = *state;
    UNUSED(mock);
    int rc;

    const char* greeting = "Hello World";
    uint8_t     buffer[BUFFER_LEN];

    NCODEC* nc = (void*)ncodec_create(MIMETYPE);
    assert_non_null(nc);

    rc = ncodec_write(nc, &(struct NCodecMessage){ .frame_id = 42,
                              .buffer = (uint8_t*)greeting,
                              .len = strlen(greeting) });
    assert_int_equal(rc, -ENOSR);
    rc = ncodec_flush(nc);
    assert_int_equal(rc, -ENOSR);

    NCodecMessage msg = {};
    rc = ncodec_read(nc, &msg);
    assert_int_equal(rc, -ENOSR);
    assert_null(msg.buffer);
    assert_int_equal(msg.len, 0);

    ncodec_close(nc);
}


void test_can_fbs_no_buffer(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    rc = ncodec_write(nc, NULL);
    assert_int_equal(rc, -EINVAL);

    rc = ncodec_read(nc, NULL);
    assert_int_equal(rc, -EINVAL);
};


void test_can_fbs_flush(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    rc = ncodec_flush(nc);
    assert_int_equal(rc, 0);
}


void test_can_fbs_write(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";

    rc = ncodec_write(nc, &(struct NCodecMessage){ .frame_id = 42,
                              .buffer = (uint8_t*)greeting,
                              .len = strlen(greeting) });
    assert_int_equal(rc, strlen(greeting));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0x66);

    // Check the result in the stream.
    stream_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    assert_int_equal(len, buffer_len);
    assert_memory_equal(buffer + 60, greeting, strlen(greeting));
}


void test_can_fbs_readwrite(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";

    // Write and flush a message.
    stream_seek(nc, 0, NCODEC_SEEK_RESET);
    rc = ncodec_write(nc, &(struct NCodecMessage){ .frame_id = 42,
                              .buffer = (uint8_t*)greeting,
                              .len = strlen(greeting) });
    assert_int_equal(rc, strlen(greeting));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0x66);


    // Seek to the start, keeping the content, modify the node_id.
    stream_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    buffer[54] = 8;
    // for (uint32_t i = 0; i< buffer_len;i+=8) printf("%02x %02x %02x %02x %02x
    // %02x %02x %02x\n",
    //     buffer[i+0], buffer[i+1], buffer[i+2], buffer[i+3],
    //     buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]);

    // Read the message back.
    NCodecMessage msg = {};
    len = ncodec_read(nc, &msg);
    assert_int_equal(len, strlen(greeting));
    assert_int_equal(msg.len, strlen(greeting));
    assert_non_null(msg.buffer);
    assert_memory_equal(msg.buffer, greeting, strlen(greeting));
}


void test_can_fbs_readwrite_frames(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting1 = "Hello World";
    const char* greeting2 = "Foo Bar";

    // Write and flush a message.
    stream_seek(nc, 0, NCODEC_SEEK_RESET);
    rc = ncodec_write(nc, &(struct NCodecMessage){ .frame_id = 42,
                              .buffer = (uint8_t*)greeting1,
                              .len = strlen(greeting1) });
    assert_int_equal(rc, strlen(greeting1));
    rc = ncodec_write(nc, &(struct NCodecMessage){ .frame_id = 42,
                              .buffer = (uint8_t*)greeting2,
                              .len = strlen(greeting2) });
    assert_int_equal(rc, strlen(greeting2));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0x92);

    // Seek to the start, keeping the content, modify the node_id.
    stream_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    buffer[58] = 8;
    buffer[58 + 40] = 8;
    // for (uint32_t i = 0; i< len;i+=8) printf("%02x %02x %02x %02x %02x %02x
    // %02x %02x\n",
    //     buffer[i+0], buffer[i+1], buffer[i+2], buffer[i+3],
    //     buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]);

    // Read the messages back.
    NCodecMessage msg = {};

    len = ncodec_read(nc, &msg);
    assert_int_equal(len, strlen(greeting1));
    assert_int_equal(msg.len, strlen(greeting1));
    assert_non_null(msg.buffer);
    assert_memory_equal(msg.buffer, greeting1, strlen(greeting1));

    len = ncodec_read(nc, &msg);
    assert_int_equal(len, strlen(greeting2));
    assert_int_equal(msg.len, strlen(greeting2));
    assert_non_null(msg.buffer);
    assert_memory_equal(msg.buffer, greeting2, strlen(greeting2));
}


void test_can_fbs_readwrite_messages(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting1 = "Hello World";
    const char* greeting2 = "Foo Bar";

    // Write and flush a message.
    stream_seek(nc, 0, NCODEC_SEEK_RESET);
    rc = ncodec_write(nc, &(struct NCodecMessage){ .frame_id = 42,
                              .buffer = (uint8_t*)greeting1,
                              .len = strlen(greeting1) });
    assert_int_equal(rc, strlen(greeting1));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0x66);

    rc = ncodec_write(nc, &(struct NCodecMessage){ .frame_id = 42,
                              .buffer = (uint8_t*)greeting2,
                              .len = strlen(greeting2) });
    assert_int_equal(rc, strlen(greeting2));
    len = ncodec_flush(nc);
    assert_int_equal(len, 0x62);

    // Seek to the start of Reset the stream, keeping the content.
    stream_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    assert_int_equal(0xc8, buffer_len);
    buffer[54] = 9;
    buffer[0x66 + 54] = 7;

    // Read the messages back.
    NCodecMessage msg = {};

    len = ncodec_read(nc, &msg);
    assert_int_equal(len, strlen(greeting1));
    assert_int_equal(msg.len, strlen(greeting1));
    assert_non_null(msg.buffer);
    assert_memory_equal(msg.buffer, greeting1, strlen(greeting1));

    len = ncodec_read(nc, &msg);
    assert_int_equal(len, strlen(greeting2));
    assert_int_equal(msg.len, strlen(greeting2));
    assert_non_null(msg.buffer);
    assert_memory_equal(msg.buffer, greeting2, strlen(greeting2));
}


int run_can_fbs_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest can_fbs_tests[] = {
        cmocka_unit_test_setup_teardown(test_can_fbs_no_stream, s, t),
        cmocka_unit_test_setup_teardown(test_can_fbs_no_buffer, s, t),
        cmocka_unit_test_setup_teardown(test_can_fbs_flush, s, t),
        cmocka_unit_test_setup_teardown(test_can_fbs_write, s, t),
        cmocka_unit_test_setup_teardown(test_can_fbs_readwrite, s, t),
        cmocka_unit_test_setup_teardown(test_can_fbs_readwrite_frames, s, t),
        cmocka_unit_test_setup_teardown(test_can_fbs_readwrite_messages, s, t),
    };

    return cmocka_run_group_tests_name("CAN FBS", can_fbs_tests, NULL, NULL);
}
