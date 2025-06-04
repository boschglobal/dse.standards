// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <testing.h>
#include <errno.h>
#include <stdio.h>
#include <dse/ncodec/codec.h>
#include <automotive-bus/codec.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define payload_LEN   1024


extern void             free_codec(ABCodecInstance* nc);
extern char*            trim(char* s);
extern void             codec_config(NCODEC* nc, NCodecConfigItem item);
extern NCodecConfigItem codec_stat(NCODEC* nc, int* index);
extern NCODEC*          ncodec_create(const char* mime_type);
extern void             codec_close(NCODEC* nc);
extern int32_t          pdu_write(NCODEC* nc, NCodecMessage* msg);
extern int32_t          pdu_read(NCODEC* nc, NCodecMessage* msg);
extern int32_t          pdu_flush(NCODEC* nc);
extern int32_t stream_read(NCODEC* nc, uint8_t** data, size_t* len, int pos_op);


typedef struct Mock {
    NCODEC* nc;
} Mock;

extern NCodecStreamVTable mem_stream;


#define MIMETYPE                                                               \
    "application/x-automotive-bus; "                                           \
    "interface=stream;type=pdu;schema=fbs;"                                    \
    "swc_id=4;ecu_id=5"
#define BUF_SWCID_OFFSET 40


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


void test_pdu_fbs_no_stream(void** state)
{
    Mock* mock = *state;
    UNUSED(mock);
    int rc;

    const char* greeting = "Hello World";
    uint8_t     payload[payload_LEN];

    NCODEC* nc = (void*)ncodec_create(MIMETYPE);
    assert_non_null(nc);

    rc = ncodec_write(nc, &(struct NCodecPdu){ .id = 42,
                              .payload = (uint8_t*)greeting,
                              .payload_len = strlen(greeting) });
    assert_int_equal(rc, -ENOSR);
    rc = ncodec_flush(nc);
    assert_int_equal(rc, -ENOSR);

    NCodecPdu pdu = {};
    rc = ncodec_read(nc, &pdu);
    assert_int_equal(rc, -ENOSR);
    assert_null(pdu.payload);
    assert_int_equal(pdu.payload_len, 0);

    ncodec_close(nc);
}


void test_pdu_fbs_no_payload(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    rc = ncodec_write(nc, NULL);
    assert_int_equal(rc, -EINVAL);

    rc = ncodec_read(nc, NULL);
    assert_int_equal(rc, -EINVAL);
};


void test_pdu_fbs_flush(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    rc = ncodec_flush(nc);
    assert_int_equal(rc, 0);
}


void test_pdu_fbs_truncate(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";

    // Write to the stream.
    ncodec_seek(nc, 0, NCODEC_SEEK_RESET);
    rc = ncodec_write(nc, &(struct NCodecPdu){ .id = 42,
                              .payload = (uint8_t*)greeting,
                              .payload_len = strlen(greeting) });
    assert_int_equal(rc, strlen(greeting));
    assert_int_equal(0x56, ncodec_flush(nc));
    assert_int_equal(0x56, ncodec_tell(nc));

    // Truncate the stream.
    rc = ncodec_truncate(nc);
    assert_int_equal(rc, 0);
    assert_int_equal(0, ncodec_tell(nc));

    // Flush the stream, and check no payloaded content was retained.
    rc = ncodec_flush(nc);
    assert_int_equal(rc, 0);
    assert_int_equal(0, ncodec_tell(nc));
}


void test_pdu_fbs_read_nomsg(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    ncodec_seek(nc, 0, NCODEC_SEEK_RESET);
    NCodecPdu pdu = {};
    size_t    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, -ENOMSG);
    assert_int_equal(pdu.payload_len, 0);
    assert_null(pdu.payload);
}


void test_pdu_fbs_write(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";

    rc = ncodec_write(nc, &(struct NCodecPdu){ .id = 42,
                              .payload = (uint8_t*)greeting,
                              .payload_len = strlen(greeting) });
    assert_int_equal(rc, strlen(greeting));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0x56);

    // Check the result in the stream.
    ncodec_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    if (0) {
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%02x %02x %02x %02x %02x %02x %02x %02x\n", buffer[i + 0],
                buffer[i + 1], buffer[i + 2], buffer[i + 3], buffer[i + 4],
                buffer[i + 5], buffer[i + 6], buffer[i + 7]);
    }
    assert_int_equal(len, buffer_len);
    assert_memory_equal(buffer + 52, greeting, strlen(greeting));
}


void test_pdu_fbs_readwrite(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";

    // Write and flush a message.
    ncodec_seek(nc, 0, NCODEC_SEEK_RESET);
    rc = ncodec_write(nc, &(struct NCodecPdu){ .id = 42,
                              .payload = (uint8_t*)greeting,
                              .payload_len = strlen(greeting) });
    assert_int_equal(rc, strlen(greeting));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0x56);

    // Seek to the start, keeping the content, modify the node_id.
    ncodec_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    buffer[BUF_SWCID_OFFSET] = 0x22;
    if (0) {
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%p: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                &(buffer[i + 0]), buffer[i + 0], buffer[i + 1], buffer[i + 2],
                buffer[i + 3], buffer[i + 4], buffer[i + 5], buffer[i + 6],
                buffer[i + 7]);
    }

    // Read the message back.
    NCodecPdu pdu = {};
    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting));
    assert_int_equal(pdu.payload_len, strlen(greeting));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting, strlen(greeting));
    assert_int_equal(pdu.swc_id, 0x22);  // Note this value was modified.
    assert_int_equal(pdu.ecu_id, 5);
}


void test_pdu_fbs_readwrite_pdus(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting1 = "Hello World";
    const char* greeting2 = "Foo Bar";

    // Write and flush a message.
    ncodec_seek(nc, 0, NCODEC_SEEK_RESET);
    rc = ncodec_write(nc, &(struct NCodecPdu){ .id = 42,
                              .payload = (uint8_t*)greeting1,
                              .payload_len = strlen(greeting1),
                              .swc_id = 42,
                              .ecu_id = 24 });
    assert_int_equal(rc, strlen(greeting1));
    ncodec_seek(nc, 0, NCODEC_SEEK_RESET);
    rc = ncodec_write(nc, &(struct NCodecPdu){ .id = 42,
                              .payload = (uint8_t*)greeting2,
                              .payload_len = strlen(greeting2),
                              .swc_id = 42,
                              .ecu_id = 24 });
    assert_int_equal(rc, strlen(greeting2));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0x7a);

    // Seek to the start, keeping the content, modify the node_id.
    ncodec_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    buffer[BUF_SWCID_OFFSET + 4] = 0x42;
    buffer[BUF_SWCID_OFFSET + 4 + 32] = 0x42;
    if (0) {
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%p: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                &(buffer[i + 0]), buffer[i + 0], buffer[i + 1], buffer[i + 2],
                buffer[i + 3], buffer[i + 4], buffer[i + 5], buffer[i + 6],
                buffer[i + 7]);
    }

    // Read the message back.
    NCodecPdu pdu = {};

    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting1));
    assert_int_equal(pdu.payload_len, strlen(greeting1));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting1, strlen(greeting1));
    assert_int_equal(pdu.swc_id, 0x42);  // Note this value was modified.
    assert_int_equal(pdu.ecu_id, 24);


    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting2));
    assert_int_equal(pdu.payload_len, strlen(greeting2));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting2, strlen(greeting2));
    assert_int_equal(pdu.swc_id, 0x42);  // Note this value was modified.
    assert_int_equal(pdu.ecu_id, 24);
}


void test_pdu_fbs_readwrite_messages(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting1 = "Hello World";
    const char* greeting2 = "Foo Bar";

    // Write and flush a message.
    ncodec_seek(nc, 0, NCODEC_SEEK_RESET);
    rc = ncodec_write(nc, &(struct NCodecPdu){ .id = 42,
                              .payload = (uint8_t*)greeting1,
                              .payload_len = strlen(greeting1),
                              .swc_id = 42,
                              .ecu_id = 24 });
    assert_int_equal(rc, strlen(greeting1));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0x56);

    rc = ncodec_write(nc, &(struct NCodecPdu){ .id = 42,
                              .payload = (uint8_t*)greeting2,
                              .payload_len = strlen(greeting2),
                              .swc_id = 42,
                              .ecu_id = 24 });
    assert_int_equal(rc, strlen(greeting2));
    len = ncodec_flush(nc);
    assert_int_equal(len, 0x52);

    // Seek to the start, keeping the content, modify the node_id.
    ncodec_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    buffer[BUF_SWCID_OFFSET] = 0x42;
    buffer[BUF_SWCID_OFFSET + 0x56] = 0x42;
    if (0) {
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%p: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                &(buffer[i + 0]), buffer[i + 0], buffer[i + 1], buffer[i + 2],
                buffer[i + 3], buffer[i + 4], buffer[i + 5], buffer[i + 6],
                buffer[i + 7]);
    }

    // Read the message back.
    NCodecPdu pdu = {};

    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting1));
    assert_int_equal(pdu.payload_len, strlen(greeting1));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting1, strlen(greeting1));
    assert_int_equal(pdu.swc_id, 0x42);  // Note this value was modified.
    assert_int_equal(pdu.ecu_id, 24);


    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting2));
    assert_int_equal(pdu.payload_len, strlen(greeting2));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting2, strlen(greeting2));
    assert_int_equal(pdu.swc_id, 0x42);  // Note this value was modified.
    assert_int_equal(pdu.ecu_id, 24);
}


typedef struct can_transport_testcase {
    NCodecPduCanFrameFormat frame_format;
    NCodecPduCanFrameType   frame_type;
    uint32_t                interface_id;
    uint32_t                network_id;
} can_transport_testcase;

void test_pdu_transport_can(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char*            greeting = "Hello World";
    can_transport_testcase tc[] = {
        { .frame_format = NCodecPduCanFrameFormatBase,
            .frame_type = NCodecPduCanFrameTypeData,
            .interface_id = 1,
            .network_id = 2 },
        { .frame_format = NCodecPduCanFrameFormatExtended,
            .frame_type = NCodecPduCanFrameTypeData,
            .interface_id = 3,
            .network_id = 4 },
        { .frame_format = NCodecPduCanFrameFormatFdBase,
            .frame_type = NCodecPduCanFrameTypeData,
            .interface_id = 5,
            .network_id = 6 },
        { .frame_format = NCodecPduCanFrameFormatFdExtended,
            .frame_type = NCodecPduCanFrameTypeData,
            .interface_id = 7,
            .network_id = 8 },
        { .frame_format = NCodecPduCanFrameFormatFdExtended,
            .frame_type = NCodecPduCanFrameTypeRemote,
            .interface_id = 9,
            .network_id = 10 },
        { .frame_format = NCodecPduCanFrameFormatFdExtended,
            .frame_type = NCodecPduCanFrameTypeError,
            .interface_id = 11,
            .network_id = 12 },
        { .frame_format = NCodecPduCanFrameFormatFdExtended,
            .frame_type = NCodecPduCanFrameTypeOverload,
            .interface_id = 13,
            .network_id = 14 },
    };

    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        // Write and flush a message.
        ncodec_truncate(nc);
        rc = ncodec_write(nc, &(struct NCodecPdu){
                                .id = 42,
                                .payload = (uint8_t*)greeting,
                                .payload_len = strlen(greeting),
                                .transport_type = NCodecPduTransportTypeCan,
                                .transport.can_message = {
                                    .frame_format = tc[i].frame_format,
                                    .frame_type = tc[i].frame_type,
                                    .interface_id = tc[i].interface_id,
                                    .network_id = tc[i].network_id,
                                },
                            });
        assert_int_equal(rc, strlen(greeting));
        size_t len = ncodec_flush(nc);
        size_t adj =
            4;  //(tc[i].frame_format ? 0 : 2) + (tc[i].frame_type ? 0: 2);
        if (tc[i].frame_format || tc[i].frame_type) adj = 0;
        assert_int_equal(len, 0x7a - adj);

        // Seek to the start, keeping the content, modify the node_id.
        ncodec_seek(nc, 0, NCODEC_SEEK_SET);
        uint8_t* buffer;
        size_t   buffer_len;
        stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
        buffer[BUF_SWCID_OFFSET] = 0x22;
        if (0) {
            for (uint32_t i = 0; i < buffer_len; i += 8)
                printf("%p: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    &(buffer[i + 0]), buffer[i + 0], buffer[i + 1],
                    buffer[i + 2], buffer[i + 3], buffer[i + 4], buffer[i + 5],
                    buffer[i + 6], buffer[i + 7]);
        }

        // Read the message back.
        NCodecPdu pdu = {};
        len = ncodec_read(nc, &pdu);
        assert_int_equal(len, strlen(greeting));
        assert_int_equal(pdu.payload_len, strlen(greeting));
        assert_non_null(pdu.payload);
        assert_memory_equal(pdu.payload, greeting, strlen(greeting));
        assert_int_equal(pdu.swc_id, 0x22);  // Note this value was modified.
        assert_int_equal(pdu.ecu_id, 5);

        // Check the transport.
        assert_int_equal(pdu.transport_type, NCodecPduTransportTypeCan);
        assert_int_equal(
            pdu.transport.can_message.frame_format, tc[i].frame_format);
        assert_int_equal(
            pdu.transport.can_message.frame_type, tc[i].frame_type);
        assert_int_equal(
            pdu.transport.can_message.interface_id, tc[i].interface_id);
        assert_int_equal(
            pdu.transport.can_message.network_id, tc[i].network_id);
    }
}

void test_pdu_transport_ip__eth(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";

    // Write and flush a message.
    ncodec_truncate(nc);
    rc = ncodec_write(nc, &(struct NCodecPdu){
        .id = 42,
        .payload = (uint8_t*)greeting,
        .payload_len = strlen(greeting),
        .transport_type = NCodecPduTransportTypeIp,
        .transport.ip_message = {
            .eth_dst_mac = 0x0000123456789ABC,
            .eth_src_mac = 0x0000CBA987654321,
            .eth_ethertype = 1,
            .eth_tci_pcp = 2,
            .eth_tci_dei = 3,
            .eth_tci_vid = 4,
        },
    });
    assert_int_equal(rc, strlen(greeting));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0x8e);

    // Seek to the start, keeping the content, modify the node_id.
    ncodec_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    buffer[BUF_SWCID_OFFSET + 4] = 0x22;
    if (0) {
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%p: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                &(buffer[i + 0]), buffer[i + 0], buffer[i + 1], buffer[i + 2],
                buffer[i + 3], buffer[i + 4], buffer[i + 5], buffer[i + 6],
                buffer[i + 7]);
    }

    // Read the message back.
    NCodecPdu pdu = {};
    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting));
    assert_int_equal(pdu.payload_len, strlen(greeting));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting, strlen(greeting));
    assert_int_equal(pdu.swc_id, 0x22);  // Note this value was modified.
    assert_int_equal(pdu.ecu_id, 5);

    // Check the transport.
    assert_int_equal(pdu.transport_type, NCodecPduTransportTypeIp);
    assert_int_equal(pdu.transport.ip_message.eth_dst_mac, 0x0000123456789ABC);
    assert_int_equal(pdu.transport.ip_message.eth_src_mac, 0x0000CBA987654321);
    assert_int_equal(pdu.transport.ip_message.eth_ethertype, 1);
    assert_int_equal(pdu.transport.ip_message.eth_tci_pcp, 2);
    assert_int_equal(pdu.transport.ip_message.eth_tci_dei, 3);
    assert_int_equal(pdu.transport.ip_message.eth_tci_vid, 4);
}

void test_pdu_transport_ip__ip(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";

    // Write and flush a message.
    ncodec_truncate(nc);
    rc = ncodec_write(nc, &(struct NCodecPdu){
                            .id = 42,
                            .swc_id = 44,
                            .payload = (uint8_t*)greeting,
                            .payload_len = strlen(greeting),
                            .transport_type = NCodecPduTransportTypeIp,
                            .transport.ip_message = {
                                .ip_protocol = NCodecPduIpProtocolTcp,
                                .ip_addr_type = NCodecPduIpAddrIPv4,
                                .ip_addr.ip_v4.src_addr = 1001,
                                .ip_addr.ip_v4.dst_addr = 2002,
                                .ip_src_port = 3003,
                                .ip_dst_port = 4004,
                            },
                        });
    assert_int_equal(rc, strlen(greeting));

    rc = ncodec_write(nc, &(struct NCodecPdu){
                            .id = 42,
                            .swc_id = 44,
                            .payload = (uint8_t*)greeting,
                            .payload_len = strlen(greeting),
                            .transport_type = NCodecPduTransportTypeIp,
                            .transport.ip_message = {
                                .ip_protocol = NCodecPduIpProtocolUdp,
                                .ip_addr_type = NCodecPduIpAddrIPv6,
                                .ip_addr.ip_v6 = {
                                    .src_addr = {1, 2, 3, 4, 5, 6, 7, 8},
                                    .dst_addr = {2, 2, 4, 4, 6, 6, 8, 8},
                                },
                                .ip_src_port = 4003,
                                .ip_dst_port = 3004,
                            },
                        });
    assert_int_equal(rc, strlen(greeting));

    size_t len = ncodec_flush(nc);
    // assert_int_equal(len, 0x8e);

    // Seek to the start, keeping the content, modify the node_id.
    ncodec_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    if (0) {
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%p: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                &(buffer[i + 0]), buffer[i + 0], buffer[i + 1], buffer[i + 2],
                buffer[i + 3], buffer[i + 4], buffer[i + 5], buffer[i + 6],
                buffer[i + 7]);
    }

    NCodecPdu pdu = {};

    // Read the next message (first)
    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting));
    assert_int_equal(pdu.payload_len, strlen(greeting));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting, strlen(greeting));
    assert_int_equal(pdu.swc_id, 44);
    assert_int_equal(pdu.ecu_id, 5);
    // Check the transport.
    assert_int_equal(pdu.transport_type, NCodecPduTransportTypeIp);
    assert_int_equal(
        pdu.transport.ip_message.ip_protocol, NCodecPduIpProtocolTcp);
    assert_int_equal(
        pdu.transport.ip_message.ip_addr_type, NCodecPduIpAddrIPv4);
    assert_int_equal(pdu.transport.ip_message.ip_addr.ip_v4.src_addr, 1001);
    assert_int_equal(pdu.transport.ip_message.ip_addr.ip_v4.dst_addr, 2002);
    assert_int_equal(pdu.transport.ip_message.ip_src_port, 3003);
    assert_int_equal(pdu.transport.ip_message.ip_dst_port, 4004);

    // Read the message back (second).
    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting));
    assert_int_equal(pdu.payload_len, strlen(greeting));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting, strlen(greeting));
    assert_int_equal(pdu.swc_id, 44);
    assert_int_equal(pdu.ecu_id, 5);
    // Check the transport.
    uint16_t src[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint16_t dst[] = { 2, 2, 4, 4, 6, 6, 8, 8 };
    assert_int_equal(pdu.transport_type, NCodecPduTransportTypeIp);
    assert_int_equal(
        pdu.transport.ip_message.ip_protocol, NCodecPduIpProtocolUdp);
    assert_int_equal(
        pdu.transport.ip_message.ip_addr_type, NCodecPduIpAddrIPv6);
    assert_memory_equal(
        pdu.transport.ip_message.ip_addr.ip_v6.src_addr, src, 16);
    assert_memory_equal(
        pdu.transport.ip_message.ip_addr.ip_v6.dst_addr, dst, 16);
    assert_int_equal(pdu.transport.ip_message.ip_src_port, 4003);
    assert_int_equal(pdu.transport.ip_message.ip_dst_port, 3004);
}

void test_pdu_transport_ip__module_do_ad(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";

    // Write and flush a message.
    ncodec_truncate(nc);
    rc = ncodec_write(nc, &(struct NCodecPdu){
                            .id = 42,
                            .swc_id = 24,
                            .payload = (uint8_t*)greeting,
                            .payload_len = strlen(greeting),
                            .transport_type = NCodecPduTransportTypeIp,
                            .transport.ip_message = {
                                .so_ad_type = NCodecPduSoAdDoIP,
                                .so_ad.do_ip = {
                                    .protocol_version = 4,
                                    .payload_type = 6,
                                },
                            },
                        });
    assert_int_equal(rc, strlen(greeting));
    size_t len = ncodec_flush(nc);

    // Seek to the start, keeping the content, modify the node_id.
    ncodec_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    if (0) {
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%p: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                &(buffer[i + 0]), buffer[i + 0], buffer[i + 1], buffer[i + 2],
                buffer[i + 3], buffer[i + 4], buffer[i + 5], buffer[i + 6],
                buffer[i + 7]);
    }

    // Read the message back.
    NCodecPdu pdu = {};
    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting));
    assert_int_equal(pdu.payload_len, strlen(greeting));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting, strlen(greeting));
    assert_int_equal(pdu.swc_id, 24);  // Note this value was modified.
    assert_int_equal(pdu.ecu_id, 5);

    // Check the transport.
    assert_int_equal(pdu.transport_type, NCodecPduTransportTypeIp);
    assert_int_equal(pdu.transport.ip_message.so_ad_type, NCodecPduSoAdDoIP);
    assert_int_equal(pdu.transport.ip_message.so_ad.do_ip.protocol_version, 4);
    assert_int_equal(pdu.transport.ip_message.so_ad.do_ip.payload_type, 6);
}

void test_pdu_transport_ip__module_some_ip(void** state)
{
    // some_ip:SomeIpMetadata;
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";

    // Write and flush a message.
    ncodec_truncate(nc);
    rc = ncodec_write(nc, &(struct NCodecPdu){
                            .id = 42,
                            .swc_id = 24,
                            .payload = (uint8_t*)greeting,
                            .payload_len = strlen(greeting),
                            .transport_type = NCodecPduTransportTypeIp,
                            .transport.ip_message = {
                                .so_ad_type = NCodecPduSoAdSomeIP,
                                .so_ad.some_ip = {
                                    .message_id = 10,
                                    .length = 11,
                                    .request_id = 12,
                                    .protocol_version = 13,
                                    .interface_version = 14,
                                    .message_type = 15,
                                    .return_code = 16,
                                },
                            },
                        });
    assert_int_equal(rc, strlen(greeting));
    size_t len = ncodec_flush(nc);

    // Seek to the start, keeping the content, modify the node_id.
    ncodec_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    if (0) {
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%p: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                &(buffer[i + 0]), buffer[i + 0], buffer[i + 1], buffer[i + 2],
                buffer[i + 3], buffer[i + 4], buffer[i + 5], buffer[i + 6],
                buffer[i + 7]);
    }

    // Read the message back.
    NCodecPdu pdu = {};
    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting));
    assert_int_equal(pdu.payload_len, strlen(greeting));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting, strlen(greeting));
    assert_int_equal(pdu.swc_id, 24);  // Note this value was modified.
    assert_int_equal(pdu.ecu_id, 5);

    // Check the transport.
    assert_int_equal(pdu.transport_type, NCodecPduTransportTypeIp);
    assert_int_equal(pdu.transport.ip_message.so_ad_type, NCodecPduSoAdSomeIP);
    assert_int_equal(pdu.transport.ip_message.so_ad.some_ip.message_id, 10);
    assert_int_equal(pdu.transport.ip_message.so_ad.some_ip.length, 11);
    assert_int_equal(pdu.transport.ip_message.so_ad.some_ip.request_id, 12);
    assert_int_equal(
        pdu.transport.ip_message.so_ad.some_ip.protocol_version, 13);
    assert_int_equal(
        pdu.transport.ip_message.so_ad.some_ip.interface_version, 14);
    assert_int_equal(pdu.transport.ip_message.so_ad.some_ip.message_type, 15);
    assert_int_equal(pdu.transport.ip_message.so_ad.some_ip.return_code, 16);
}

void test_pdu_transport_struct(void** state)
{
    Mock*   mock = *state;
    NCODEC* nc = mock->nc;
    int     rc;

    const char* greeting = "Hello World";
    const char* type_name = "foo";
    const char* var_name = "bar";
    const char* encoding = "foobar";
    uint16_t    attribute_aligned = 16;
    bool        attribute_packed = true;
    const char* platform_arch = "amd64";
    const char* platform_os = "linux";
    const char* platform_abi = "abc";

    // Write and flush a message.
    ncodec_truncate(nc);
    rc = ncodec_write(nc, &(struct NCodecPdu){
        .id = 42,
        .payload = (uint8_t*)greeting,
        .payload_len = strlen(greeting),
        .transport_type = NCodecPduTransportTypeStruct,
        .transport.struct_object = {
            .type_name = type_name,
            .var_name = var_name,
            .encoding = encoding,
            .attribute_aligned = attribute_aligned,
            .attribute_packed = attribute_packed,
            .platform_arch = platform_arch,
            .platform_os = platform_os,
            .platform_abi = platform_abi,
        },
    });
    assert_int_equal(rc, strlen(greeting));
    size_t len = ncodec_flush(nc);
    assert_int_equal(len, 0xce);

    // Seek to the start, keeping the content, modify the node_id.
    ncodec_seek(nc, 0, NCODEC_SEEK_SET);
    uint8_t* buffer;
    size_t   buffer_len;
    stream_read(nc, &buffer, &buffer_len, NCODEC_POS_NC);
    buffer[BUF_SWCID_OFFSET] = 0x22;
    if (0) {
        for (uint32_t i = 0; i < buffer_len; i += 8)
            printf("%p: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                &(buffer[i + 0]), buffer[i + 0], buffer[i + 1], buffer[i + 2],
                buffer[i + 3], buffer[i + 4], buffer[i + 5], buffer[i + 6],
                buffer[i + 7]);
    }

    // Read the message back.
    NCodecPdu pdu = {};
    len = ncodec_read(nc, &pdu);
    assert_int_equal(len, strlen(greeting));
    assert_int_equal(pdu.payload_len, strlen(greeting));
    assert_non_null(pdu.payload);
    assert_memory_equal(pdu.payload, greeting, strlen(greeting));
    assert_int_equal(pdu.swc_id, 0x22);  // Note this value was modified.
    assert_int_equal(pdu.ecu_id, 5);

    // Check the transport.
    assert_string_equal(pdu.transport.struct_object.type_name, type_name);
    assert_string_equal(pdu.transport.struct_object.var_name, var_name);
    assert_string_equal(pdu.transport.struct_object.encoding, encoding);
    assert_int_equal(
        pdu.transport.struct_object.attribute_aligned, attribute_aligned);
    assert_int_equal(
        pdu.transport.struct_object.attribute_packed, attribute_packed);
    assert_string_equal(pdu.transport.struct_object.platform_arch, platform_arch);
    assert_string_equal(pdu.transport.struct_object.platform_os, platform_os);
    assert_string_equal(pdu.transport.struct_object.platform_abi, platform_abi);
}


int run_pdu_fbs_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest pdu_fbs_tests[] = {
        cmocka_unit_test_setup_teardown(test_pdu_fbs_no_stream, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_fbs_no_payload, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_fbs_flush, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_fbs_read_nomsg, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_fbs_write, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_fbs_readwrite, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_fbs_truncate, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_fbs_readwrite_pdus, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_fbs_readwrite_messages, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_transport_can, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_transport_ip__eth, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_transport_ip__ip, s, t),
        cmocka_unit_test_setup_teardown(
            test_pdu_transport_ip__module_do_ad, s, t),
        cmocka_unit_test_setup_teardown(
            test_pdu_transport_ip__module_some_ip, s, t),
        cmocka_unit_test_setup_teardown(test_pdu_transport_struct, s, t),
    };

    return cmocka_run_group_tests_name("PDU FBS", pdu_fbs_tests, NULL, NULL);
}
